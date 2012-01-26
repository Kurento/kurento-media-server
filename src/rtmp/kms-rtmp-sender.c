#include <kms-core.h>
#include <rtmp/kms-rtmp-sender.h>
#include <rtmp/kms-rtmp-session.h>
#include "internal/kms-utils.h"

#define KMS_RTMP_SENDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTMP_SENDER, KmsRtmpSenderPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTMP_SENDER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTMP_SENDER(obj)->priv->mutex))

#define VIDEO_CAPS "video/x-flash-video;"
#define AUDIO_CAPS "audio/x-nellymoser,channels=1,rate=11025;"

struct _KmsRtmpSenderPriv {
	GMutex *mutex;

	KmsRtmpSession *neg_spec;
	gboolean offerer;

	GstElement *flvmux;
};

enum {
	PROP_0,

	PROP_NEG_SPEC,
	PROP_OFFERER,
};

G_DEFINE_TYPE(KmsRtmpSender, kms_rtmp_sender, KMS_TYPE_MEDIA_HANDLER_SINK)

static void
dispose_neg_spec(KmsRtmpSender *self) {
	if (self->priv->neg_spec != NULL) {
		g_object_unref(self->priv->neg_spec);
		self->priv->neg_spec = NULL;
	}
}

static void
dispose_flvmux(KmsRtmpSender *self) {
	if (self->priv->flvmux != NULL) {
		g_object_unref(self->priv->flvmux);
		self->priv->flvmux = NULL;
	}
}

static void
unlinked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);

	KMS_DEBUG_PIPE("unlinked_rtmp");
}

static void
found_audio(GstElement *tf, guint prob, GstCaps *caps, KmsRtmpSender *self) {
	GstElement *flvmux;
	GstPad *sink, *src;

	flvmux = self->priv->flvmux;

	if (flvmux == NULL)
		return;

	/* TODO: search on templates to request a pad */
	sink = gst_element_get_request_pad(flvmux, "audio");
	src = gst_element_get_static_pad(tf, "src");

	if (src == NULL || sink == NULL) {
		g_warn_if_reached();

		if (src != NULL)
			g_object_unref(src);

		if (sink != NULL)
			gst_element_release_request_pad(flvmux, sink);

		return;
	}
	gst_pad_link(src, sink);

	g_object_unref(src);
	g_object_unref(sink);
}

static void
audio_linked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	GstElement *typefind;
	GstPad *sink;

	typefind = gst_element_factory_make("typefind", NULL);
	if (typefind == NULL)
		return;

	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), typefind);

	g_object_connect(typefind, "signal::have-type", found_audio, self, NULL);

	sink = gst_element_get_static_pad(typefind, "sink");
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), sink);

	/* TODO: Add callback to remove when unlinked */

	g_object_unref(sink);
}

static void
found_video(GstElement *tf, guint prob, GstCaps *caps, KmsRtmpSender *self) {
	GstElement *flvmux;
	GstPad *sink, *src;

	KMS_DEBUG;
	flvmux = self->priv->flvmux;

	if (flvmux == NULL)
		return;

	/* TODO: search on templates to request a pad */
	sink = gst_element_get_request_pad(flvmux, "video");
	src = gst_element_get_static_pad(tf, "src");

	if (src == NULL || sink == NULL) {
		g_warn_if_reached();

		if (src != NULL)
			g_object_unref(src);

		if (sink != NULL)
			gst_element_release_request_pad(flvmux, sink);

		return;
	}
	gst_pad_link(src, sink);

	g_object_unref(src);
	g_object_unref(sink);
}

static void
video_linked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	GstElement *typefind;
	GstPad *sink;

	KMS_DEBUG;
	typefind = gst_element_factory_make("typefind", NULL);
	if (typefind == NULL)
		return;

	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), typefind);

	g_object_connect(typefind, "signal::have-type", found_video, self, NULL);

	sink = gst_element_get_static_pad(typefind, "sink");
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), sink);

	/* TODO: Add callback to remove when unlinked */

	g_object_unref(sink);
}

static void
create_media_chain(KmsRtmpSender *self) {
	GstElement *rtmpsink, *flvmux, *queue;
	GstPadTemplate *audio_templ, *video_templ;
	GstPad *audio_pad, *video_pad;
	gchar *url;

	url = kms_rtmp_session_get_url(self->priv->neg_spec,
							!self->priv->offerer);

	if (url == NULL) {
		g_warn_if_reached();
		return;
	}

	rtmpsink = gst_element_factory_make("rtmpsink", NULL);
	flvmux = gst_element_factory_make("flvmux", NULL);
	queue = kms_utils_create_queue(NULL);

	if (rtmpsink == NULL || flvmux == NULL || queue == NULL) {
		g_warn_if_reached();

		if (rtmpsink == NULL)
			g_object_unref(rtmpsink);

		if (flvmux != NULL)
			g_object_unref(flvmux);

		if (queue != NULL)
			g_object_unref(queue);

		g_free(url);
		return;
	}

	g_object_set(flvmux, "streamable", TRUE, NULL);
	g_object_set(rtmpsink, "sync", FALSE, NULL);
// 	g_object_set(rtmpsink, "enable-last-buffer", FALSE, NULL);
	g_object_set(rtmpsink, "blocksize", 10, NULL);

	self->priv->flvmux = g_object_ref(flvmux);
	kms_utils_release_unlinked_pads(flvmux);

	g_object_set(rtmpsink, "location", url, NULL);

	gst_element_set_state(rtmpsink, GST_STATE_PLAYING);
	gst_element_set_state(flvmux, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), rtmpsink, queue, flvmux, NULL);
	gst_element_link_many(flvmux, queue, rtmpsink, NULL);

	g_free(url);

	/* TODO: Get caps from the muxer template */
	audio_templ = gst_pad_template_new("audio_sink",
					GST_PAD_SINK,
					GST_PAD_ALWAYS,
					gst_caps_from_string(AUDIO_CAPS));
	audio_pad = gst_ghost_pad_new_no_target_from_template("audio_sink",
								audio_templ);
	if (audio_pad != NULL) {
		gst_pad_set_active(audio_pad, TRUE);
		g_object_connect(audio_pad, "signal::unlinked", unlinked,
								self, NULL);
		g_object_connect(audio_pad, "signal::linked", audio_linked,
								self, NULL);
		gst_element_add_pad(GST_ELEMENT(self), audio_pad);
	}
	g_object_unref(audio_templ);

	video_templ = gst_pad_template_new("video_sink",
					GST_PAD_SINK,
					GST_PAD_ALWAYS,
					gst_caps_from_string(VIDEO_CAPS));
	video_pad = gst_ghost_pad_new_no_target_from_template("video_sink",
								video_templ);
	if (video_pad != NULL) {
		gst_pad_set_active(video_pad, TRUE);
		g_object_connect(video_pad, "signal::unlinked", unlinked,
								self, NULL);
		g_object_connect(video_pad, "signal::linked", video_linked,
								self, NULL);
		gst_element_add_pad(GST_ELEMENT(self), video_pad);
	}
	g_object_unref(video_templ);
}

static void
constructed(GObject *object) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->constructed(object);

	g_return_if_fail(self->priv->neg_spec != NULL);

	create_media_chain(self);
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	switch (property_id) {
		case PROP_NEG_SPEC:
			LOCK(self);
			dispose_neg_spec(self);
			self->priv->neg_spec = g_value_dup_object(value);
			UNLOCK(self);
			break;
		case PROP_OFFERER:
			LOCK(self);
			self->priv->offerer = g_value_get_boolean(value);
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
		case PROP_OFFERER:
			LOCK(self);
			g_value_set_boolean(value, self->priv->offerer);
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
	dispose_neg_spec(self);
	dispose_flvmux(self);
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
					KMS_TYPE_RTMP_SESSION,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_NEG_SPEC, pspec);

	pspec = g_param_spec_boolean("offerer", "Offerer",
					"If local party is offerer",
					FALSE, G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_OFFERER, pspec);
}

static void
kms_rtmp_sender_init(KmsRtmpSender *self) {
	self->priv = KMS_RTMP_SENDER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->neg_spec = NULL;
	self->priv->offerer = FALSE;
	self->priv->flvmux = NULL;
}
