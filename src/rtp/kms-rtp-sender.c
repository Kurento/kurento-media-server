#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"

#define KMS_RTP_SENDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_SENDER, KmsRtpSenderPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTP_SENDER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTP_SENDER(obj)->priv->mutex))

struct _KmsRtpSenderPriv {
	GMutex *mutex;

	KmsSdpSession *remote_spec;
	gint audio_fd;
	gint video_fd;
};

enum {
	PROP_0,

	PROP_REMOTE_SPEC,
	PROP_AUDIO_FD,
	PROP_VIDEO_FD,
};

G_DEFINE_TYPE(KmsRtpSender, kms_rtp_sender, KMS_TYPE_MEDIA_HANDLER_SINK)

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
		type_str = g_strdup("audio");
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		type_str = g_strdup("video");
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

static void
create_udpsink(KmsRtpSender *self, gchar *addr, KmsSdpMedia *media, gint fd) {
	GValueArray *payloads;
	KmsSdpPayload *payload;
	GstElement *udpsink, *payloader;
	GstCaps *caps, *sink_caps, *pay_sink_caps;
	gint port;
	KmsMediaType type;
	GstPad *pad, *sink_pad, *pay_sink;
	GstPadTemplate *templ;

	g_object_get(media, "payloads", &payloads, "port", &port, "type", &type,
									NULL);

	if (payloads->n_values == 0 || port == 0)
		goto end;

	payload = g_value_get_object(g_value_array_get_nth(payloads, 0));
	caps = kms_sdp_payload_to_caps(payload);

	payloader = kms_utils_get_element_for_caps(
					GST_ELEMENT_FACTORY_TYPE_PAYLOADER,
					GST_RANK_NONE, caps, GST_PAD_SRC,
					FALSE, NULL);
	udpsink = gst_element_factory_make("udpsink", NULL);

	if (payloader == NULL || udpsink == NULL) {
		if (payload != NULL)
			g_object_unref(payload);
		if (udpsink != NULL)
			g_object_unref(udpsink);

		goto end;
	}

	kms_utils_configure_element(payloader);
	pay_sink = gst_element_get_static_pad(payloader, "sink");
	pay_sink_caps = gst_pad_get_caps(pay_sink);
	kms_utils_transfer_caps(caps, pay_sink_caps);
	payloader = kms_generate_bin_with_caps(payloader, pay_sink_caps, caps);
	gst_caps_unref(pay_sink_caps);
	g_object_unref(pay_sink);

	g_object_set(udpsink, "host", addr, "port", port, "sync", FALSE, NULL);
	if (fd != -1)
		g_object_set(udpsink, "sockfd", fd, "closefd", FALSE, NULL);

	gst_element_set_state(udpsink, GST_STATE_PLAYING);
	gst_element_set_state(payloader, GST_STATE_PLAYING);
	gst_bin_add_many(GST_BIN(self), payloader, udpsink, NULL);
	gst_element_link(payloader, udpsink);

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
end:
	g_value_array_free(payloads);
}

static void
constructed(GObject *object) {
	KmsRtpSender *self = KMS_RTP_SENDER(object);
	GValueArray *medias;
	gchar *remote_addr;
	gint i;

	G_OBJECT_CLASS(kms_rtp_sender_parent_class)->constructed(object);

	g_return_if_fail(self->priv->remote_spec != NULL);

	g_object_get(self->priv->remote_spec, "medias", &medias,
						"address", &remote_addr, NULL);

	for (i = 0; i < medias->n_values; i++) {
		GValue *val;
		KmsSdpMedia *media;
		KmsMediaType type;

		val = g_value_array_get_nth(medias, i);
		media = g_value_get_object(val);

		g_object_get(media, "type", &type, NULL);
		switch (type) {
			case KMS_MEDIA_TYPE_AUDIO:
				create_udpsink(self, remote_addr, media,
							self->priv->audio_fd);
				break;
			case KMS_MEDIA_TYPE_VIDEO:
				create_udpsink(self, remote_addr, media,
							self->priv->video_fd);
				break;
			default:
				break;
		}
	}

	g_value_array_free(medias);
	g_free(remote_addr);
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtpSender *self = KMS_RTP_SENDER(object);

	switch (property_id) {
		case PROP_REMOTE_SPEC:
			LOCK(self);
			dispose_remote_spec(self);
			self->priv->remote_spec = g_value_dup_object(value);
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
					KMS_TYPE_SDP_SESSION,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_REMOTE_SPEC, pspec);

	pspec = g_param_spec_int("audio-fd", "Audio fd",
					"File descriptor used to send audio",
					-1, G_MAXINT, -1,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_AUDIO_FD, pspec);

	pspec = g_param_spec_int("video-fd", "Video fd",
					"File descriptor used to send video",
					-1, G_MAXINT, -1,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_VIDEO_FD, pspec);
}

static void
kms_rtp_sender_init(KmsRtpSender *self) {
	self->priv = KMS_RTP_SENDER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->remote_spec = NULL;
	self->priv->audio_fd = -1;
	self->priv->video_fd = -1;
}
