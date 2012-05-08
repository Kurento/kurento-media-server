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
#include <rtmp/kms-rtmp-sender.h>
#include "internal/kms-utils.h"
#include <kms_session_spec_types.h>

#define KMS_RTMP_SENDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTMP_SENDER, KmsRtmpSenderPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTMP_SENDER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTMP_SENDER(obj)->priv->mutex))

#define URL_DATA "url"

#define AUDIO_TEMPL "audio_sink"
#define VIDEO_TEMPL "video_sink"

#define VIDEO_CAPS "video/x-flash-video;"					\
			"video/x-flash-screen;"					\
			"video/x-vp6-flash;"					\
			"video/x-vp6-alpha;"					\
			"video/x-h264,stream-format=avc"			\

#define AUDIO_CAPS "audio/x-nellymoser,channels=1,rate=8000;"

/*
		"audio/x-adpcm,layout=swf,channels={ 1, 2 },"			\
					"rate:={ 5512, 11025, 22050, 44100 };"	\
		"audio/mpeg,mpegversion=1,layer=3,channels={ 1, 2 },"		\
			"rate={ 5512, 8000, 11025, 22050, 44100 },parsed=true;"	\
		"audio/mpeg,mpegversion=4,framed=true;"				\
		"audio/x-nellymoser,channels={ 1, 2 },"				\
			"rate={ 5512, 8000, 11025, 16000, 22050, 44100 };"	\
		"audio/x-alaw,channels={ 1, 2 },"				\
				"rate={ 5512, 11025, 22050, 44100 };"		\
		"audio/x-mulaw,channels={ 1, 2 },"				\
				"rate={ 5512, 11025, 22050, 44100 }"		\
*/

struct _KmsRtmpSenderPriv {
	GMutex *mutex;

	KmsSessionSpec *neg_spec;
};

enum {
	PROP_0,

	PROP_NEG_SPEC,
};

G_DEFINE_TYPE(KmsRtmpSender, kms_rtmp_sender, KMS_TYPE_MEDIA_HANDLER_SINK)

G_LOCK_DEFINE_STATIC(rtmp_sender_lock);

static void
dispose_local_spec(KmsRtmpSender *self) {
	if (self->priv->neg_spec != NULL) {
		g_object_unref(self->priv->neg_spec);
		self->priv->neg_spec = NULL;
	}
}

static GstElement*
create_rtmpsink(KmsRtmpSender *self, const gchar *url) {
	GstElement *rtmpsink, *queue, *flvmux;

	rtmpsink = gst_element_factory_make("rtmpsink", NULL);
	queue = kms_utils_create_queue("queue");
	flvmux = gst_element_factory_make("flvmux", NULL);

	if (queue == NULL || rtmpsink == NULL || flvmux == NULL) {
		if (queue != NULL)
			gst_object_unref(queue);

		queue = NULL;

		if (rtmpsink != NULL)
			gst_object_unref(rtmpsink);

		rtmpsink = NULL;

		if (flvmux != NULL)
			gst_object_unref(flvmux);

		flvmux = NULL;

		goto end;
	}

	g_object_set(flvmux, "streamable", TRUE, NULL);
	g_object_set(rtmpsink, "sync", FALSE, NULL);
	g_object_set(rtmpsink, "async", FALSE, NULL);
	g_object_set(rtmpsink, "enable-last-buffer", FALSE, NULL);
	g_object_set(rtmpsink, "blocksize", 10, NULL);
	g_object_set(rtmpsink, "location", url, NULL);

	G_LOCK(rtmp_sender_lock);
	gst_element_set_state(rtmpsink, GST_STATE_PLAYING);
	G_UNLOCK(rtmp_sender_lock);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(flvmux, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), rtmpsink, queue, flvmux, NULL);

	kms_utils_remove_when_unlinked_pad_name(rtmpsink, "sink");
	kms_utils_remove_when_unlinked_pad_name(queue, "sink");
	kms_utils_remove_when_unlinked_pad_name(flvmux, "audio");
	kms_utils_remove_when_unlinked_pad_name(flvmux, "video");

	gst_element_link_many(flvmux, queue, rtmpsink, NULL);

end:
	return flvmux;
}

static void
unlinked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);
}

static void
found_media(GstElement *tf, guint prob, GstCaps *caps, KmsRtmpSender *self) {
	GstElement *queue, *rtmpsink;
	gchar *url;

	url = g_object_get_data(G_OBJECT(tf), URL_DATA);

	queue = kms_utils_create_queue(NULL);

	kms_utils_remove_when_unlinked_pad_name(queue, "sink");
	kms_utils_remove_when_unlinked_pad_name(queue, "src");

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), queue);

	gst_element_link(tf, queue);

	rtmpsink = create_rtmpsink(self, url);

	if (rtmpsink != NULL)
		gst_element_link(queue, rtmpsink);
	// TODO: In case of error, unlink typefind and notify error
}

static void
linked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	GstElement *typefind;
	GstPad *sink;
	gchar *url;

	typefind = gst_element_factory_make("typefind", NULL);
	if (typefind == NULL)
		return;

	url = g_object_get_data(G_OBJECT(pad), URL_DATA);
	if (url != NULL)
		g_object_set_data_full(G_OBJECT(typefind), URL_DATA,
							g_strdup(url), g_free);

	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), typefind);

	g_object_connect(typefind, "signal::have-type", found_media, self, NULL);

	kms_utils_remove_when_unlinked_pad_name(typefind, "src");
	sink = gst_element_get_static_pad(typefind, "sink");
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), sink);
	kms_utils_remove_when_unlinked(sink);

	g_object_unref(sink);
}

static gchar*
kms_get_publish_url_from_media(KmsMediaSpec *media) {
	KmsTransportRtmp *rtmp;
	gchar *url;
	gchar *stream;
	gchar *uri;

	g_return_val_if_fail(KMS_IS_MEDIA_SPEC(media), NULL);

	if (!media->transport->__isset_rtmp)
		return NULL;

	rtmp = media->transport->rtmp;
	url = rtmp->url;

	g_return_val_if_fail(url != NULL, NULL);

	stream = rtmp->publish;

	if (stream == NULL)
		return NULL;

	uri = g_strdup_printf("%s/%s", url, stream);

	return uri;
}

static void
create_media_chain(KmsRtmpSender *self, KmsMediaSpec *media) {
	GstPadTemplate *template;
	GstPad *pad;
	gchar *url;
	gchar *caps, *templ_name;

	url = kms_get_publish_url_from_media(media);

	if (url == NULL) {
		g_warn_if_reached();
		return;
	}

	/* TODO: Get caps from flvmux factory */
	/* TODO: Get caps from mediaspec */
	if (g_hash_table_lookup(media->type, (gpointer) KMS_MEDIA_TYPE_AUDIO)) {
		caps = AUDIO_CAPS;
		templ_name = AUDIO_TEMPL;
	} else if (g_hash_table_lookup(media->type, (gpointer)
							KMS_MEDIA_TYPE_VIDEO)) {
		caps = VIDEO_CAPS;
		templ_name = VIDEO_TEMPL;
	} else {
		g_warn_if_reached();
		return;
	}


	template = gst_pad_template_new(templ_name,
					GST_PAD_SINK,
					GST_PAD_ALWAYS,
					gst_caps_from_string(caps));
	pad = gst_ghost_pad_new_no_target_from_template(templ_name, template);
	if (pad != NULL) {
		gst_pad_set_active(pad, TRUE);
		g_object_connect(pad, "signal::unlinked", unlinked, self, NULL);
		g_object_connect(pad, "signal::linked", linked, self, NULL);
		g_object_set_data_full(G_OBJECT(pad), URL_DATA, url, g_free);

		gst_element_add_pad(GST_ELEMENT(self), pad);
	}

	g_object_unref(template);
}

static void
constructed(GObject *object) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);
	gint i;

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->constructed(object);

	g_return_if_fail(self->priv->neg_spec != NULL);

	for (i = 0; i < self->priv->neg_spec->medias->len; i ++) {
		KmsMediaSpec *media;

		media = self->priv->neg_spec->medias->pdata[i];
		create_media_chain(self, media);
	}
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	switch (property_id) {
		case PROP_NEG_SPEC:
			LOCK(self);
			dispose_local_spec(self);
			self->priv->neg_spec = g_value_dup_object(value);
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
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	switch (property_id) {
		case PROP_NEG_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->neg_spec);
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
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	LOCK(self);
	dispose_local_spec(self);
	UNLOCK(self);

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	g_mutex_free(self->priv->mutex);

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->finalize(object);
}

static void
kms_rtmp_sender_class_init(KmsRtmpSenderClass *klass) {
	GParamSpec *pspec;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsRtmpSenderPriv));

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
		GST_ELEMENT_CLASS(kms_rtmp_sender_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_rtmp_sender_parent_class)->numpadtemplates;

	pspec = g_param_spec_object("neg-spec", "Negotiated Session Spec",
					"Negotiated Session Description",
					KMS_TYPE_SESSION_SPEC,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_NEG_SPEC, pspec);
}

static void
kms_rtmp_sender_init(KmsRtmpSender *self) {
	self->priv = KMS_RTMP_SENDER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->neg_spec = NULL;
}
