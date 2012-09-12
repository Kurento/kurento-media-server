/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"
#include <nice.h>

#define KMS_RTP_SENDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_SENDER, KmsRtpSenderPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTP_SENDER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTP_SENDER(obj)->priv->mutex))

#define STREAM_ID_DATA "stream_id"

struct _KmsRtpSenderPriv {
	GMutex *mutex;

	KmsSessionSpec *remote_spec;
	gint audio_fd;
	gint video_fd;

	NiceAgent *audio_agent;
	NiceAgent *video_agent;
};

enum {
	PROP_0,

	PROP_REMOTE_SPEC,
	PROP_AUDIO_FD,
	PROP_VIDEO_FD,
	PROP_AUDIO_AGENT,
	PROP_VIDEO_AGENT,
};

G_DEFINE_TYPE(KmsRtpSender, kms_rtp_sender, KMS_TYPE_MEDIA_HANDLER_SINK)

static void
dispose_video_agent(KmsRtpSender *self) {
	if (self->priv->video_agent != NULL) {
		g_object_unref(self->priv->video_agent);
		self->priv->video_agent = NULL;
	}
}

static void
dispose_audio_agent(KmsRtpSender *self) {
	if (self->priv->audio_agent != NULL) {
		g_object_unref(self->priv->audio_agent);
		self->priv->audio_agent = NULL;
	}
}

static void
dispose_remote_spec(KmsRtpSender *self) {
	if (self->priv->remote_spec != NULL) {
		g_object_unref(self->priv->remote_spec);
		self->priv->remote_spec = NULL;
	}
}

static GstPadTemplate*
create_pad_template(KmsRtpSender *self, const GstCaps *caps, KmsMediaType type) {
	GList *templates, *l;
	gchar *type_str;
	GstPadTemplate *templ = NULL;

	templates = gst_element_class_get_pad_template_list(
						GST_ELEMENT_GET_CLASS(self));
	l = templates;
	switch(type) {
	case KMS_MEDIA_TYPE_AUDIO:
		type_str = g_strdup("audio_sink");
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		type_str = g_strdup("video_sink");
		break;
	default:
		return NULL;
	}

	while (l != NULL) {
		GstPadTemplate *t;

		t = l->data;
		if (g_strstr_len(t->name_template, -1, type_str) != NULL) {
			templ = gst_pad_template_new(t->name_template,
							t->direction,
							t->presence,
							gst_caps_copy(caps));
			goto end;
		}
		l = l->next;
	}

end:
	g_free(type_str);

	return templ;
}

static gint
get_address_port(KmsMediaSpec *media, gchar **addr) {
	if (!media->transport->__isset_rtp || media->transport->rtp == NULL)
		return 0;

	*addr = media->transport->rtp->address;
	if (*addr == NULL)
		return 0;

	return media->transport->rtp->port;
}

static void
create_udpsink(KmsRtpSender *self, KmsMediaSpec *media) {
	NiceAgent *agent;
	KmsPayload *payload;
	GstElement *networksink, *payloader;
	GstCaps *caps, *sink_caps, *pay_sink_caps;
	gint port;
	KmsMediaType type;
	GstPad *pad, *sink_pad, *pay_sink;
	GstPadTemplate *templ;
	gchar *addr;
	gint fd;

	port = get_address_port(media, &addr);

	if (media->payloads->len == 0)
		return;

	if (media->direction == KMS_DIRECTION_INACTIVE ||
				media->direction == KMS_DIRECTION_SENDONLY)
		return;

	if (g_hash_table_lookup(media->type, (gpointer) KMS_MEDIA_TYPE_AUDIO)) {
		type = KMS_MEDIA_TYPE_AUDIO;
		fd = self->priv->audio_fd;
		agent = self->priv->audio_agent;
	} else if (g_hash_table_lookup(media->type,
					(gpointer) KMS_MEDIA_TYPE_VIDEO)) {
		type = KMS_MEDIA_TYPE_VIDEO;
		fd = self->priv->video_fd;
		agent = self->priv->video_agent;
	} else {
		return;
	}

	if (agent == NULL && port == 0)
		return;

	payload = media->payloads->pdata[0];
	caps = kms_payload_to_caps(payload, media);

	payloader = kms_utils_get_element_for_caps(
					GST_ELEMENT_FACTORY_TYPE_PAYLOADER,
					GST_RANK_NONE, caps, GST_PAD_SRC,
					FALSE, NULL);

	if (agent == NULL)
		networksink = gst_element_factory_make("udpsink", NULL);
	else
		networksink = gst_element_factory_make("nicesink", NULL);

	if (payloader == NULL || networksink == NULL) {
		if (payload != NULL)
			g_object_unref(payload);
		if (networksink != NULL)
			g_object_unref(networksink);

		return;
	}

	kms_utils_configure_element(payloader);
	pay_sink = gst_element_get_static_pad(payloader, "sink");
	pay_sink_caps = gst_pad_get_caps(pay_sink);
	kms_utils_transfer_caps(caps, pay_sink_caps);
	payloader = kms_generate_bin_with_caps(payloader, pay_sink_caps, caps);
	gst_caps_unref(pay_sink_caps);
	g_object_unref(pay_sink);

	if (agent == NULL) {
		g_object_set(networksink, "host", addr, "port", port,
							"sync", FALSE, NULL);
		if (fd != -1)
			g_object_set(networksink, "sockfd", fd,
							"closefd", FALSE, NULL);
	} else {
		guint stream_id;

		stream_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(agent),
							STREAM_ID_DATA));
		g_object_set(networksink, "agent", agent, "stream", stream_id,
						"component", (guint) 1, NULL);
	}

	gst_element_set_state(networksink, GST_STATE_PLAYING);
	gst_element_set_state(payloader, GST_STATE_PLAYING);
	gst_bin_add_many(GST_BIN(self), payloader, networksink, NULL);
	gst_element_link(payloader, networksink);

	sink_pad = gst_element_get_static_pad(payloader, "sink");
	sink_caps = gst_pad_get_caps(sink_pad);
	templ = create_pad_template(self, sink_caps, type);
	if (templ != NULL) {
		pad = gst_ghost_pad_new_from_template(templ->name_template,
							sink_pad, templ);
		if (pad != NULL) {
			gst_pad_set_active(pad, TRUE);
			/* TODO: Possibly connect callbacks to pad */
			gst_element_add_pad(GST_ELEMENT(self), pad);
		}
		g_object_unref(templ);
	}
	gst_caps_unref(sink_caps);
	g_object_unref(sink_pad);
}

static void
start_media(KmsRtpSender *self) {
	KmsSessionSpec *spec = self->priv->remote_spec;
	gint i;

	g_assert(self->priv->remote_spec != NULL);

	for (i = 0; i < spec->medias->len; i++) {
		KmsMediaSpec *media;

		media = spec->medias->pdata[i];

		create_udpsink(self, media);
	}
}

static void
constructed(GObject *object) {
	G_OBJECT_CLASS(kms_rtp_sender_parent_class)->constructed(object);

}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtpSender *self = KMS_RTP_SENDER(object);

	switch (property_id) {
		case PROP_REMOTE_SPEC:
			LOCK(self);
			if (self->priv->remote_spec != NULL) {
				g_warn_if_reached();
				UNLOCK(self);
				return;
			}
			dispose_remote_spec(self);
			self->priv->remote_spec = g_value_dup_object(value);
			if (self->priv->remote_spec != NULL)
				start_media(self);
			UNLOCK(self);
			break;
		case PROP_AUDIO_FD:
			LOCK(self);
			self->priv->audio_fd = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_VIDEO_FD:
			LOCK(self);
			self->priv->video_fd = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_AUDIO_AGENT:
			LOCK(self);
			dispose_audio_agent(self);
			self->priv->audio_agent = g_value_dup_object(value);
			UNLOCK(self);
			break;
		case PROP_VIDEO_AGENT:
			LOCK(self);
			dispose_video_agent(self);
			self->priv->video_agent = g_value_dup_object(value);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsRtpSender *self = KMS_RTP_SENDER(object);

	switch (property_id) {
		case PROP_REMOTE_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->remote_spec);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
dispose(GObject *object) {
	KmsRtpSender *self = KMS_RTP_SENDER(object);

	LOCK(self);
	dispose_remote_spec(self);
	dispose_audio_agent(self);
	dispose_video_agent(self);
	UNLOCK(self);

	G_OBJECT_CLASS(kms_rtp_sender_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtpSender *self = KMS_RTP_SENDER(object);

	g_mutex_free(self->priv->mutex);

	G_OBJECT_CLASS(kms_rtp_sender_parent_class)->finalize(object);
}

static void
kms_rtp_sender_class_init(KmsRtpSenderClass *klass) {
	GParamSpec *pspec;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsRtpSenderPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->constructed = constructed;

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_rtp_sender_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_rtp_sender_parent_class)->numpadtemplates;

	pspec = g_param_spec_object("remote-spec", "Remote Session Spec",
					"Remote Session Description",
					KMS_TYPE_SESSION_SPEC,
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_REMOTE_SPEC, pspec);

	pspec = g_param_spec_int("audio-fd", "Audio fd",
					"File descriptor used to send audio",
					-1, G_MAXINT, -1,
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_AUDIO_FD, pspec);

	pspec = g_param_spec_int("video-fd", "Video fd",
					"File descriptor used to send video",
					-1, G_MAXINT, -1,
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_VIDEO_FD, pspec);

	pspec = g_param_spec_object("audio-agent", "ICE Audio Agent",
				    "ICE Audio Agent",
			     NICE_TYPE_AGENT,
			     G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_AUDIO_AGENT, pspec);

	pspec = g_param_spec_object("video-agent", "ICE Video Agent",
				    "ICE Video Agent",
			     NICE_TYPE_AGENT,
			     G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_VIDEO_AGENT, pspec);
}

static void
kms_rtp_sender_init(KmsRtpSender *self) {
	self->priv = KMS_RTP_SENDER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->remote_spec = NULL;
	self->priv->audio_fd = -1;
	self->priv->video_fd = -1;
	self->priv->audio_agent = NULL;
	self->priv->video_agent = NULL;
}
