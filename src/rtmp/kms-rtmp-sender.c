#include <kms-core.h>
#include <rtmp/kms-rtmp-sender.h>
#include <rtmp/kms-rtmp-session.h>
#include "internal/kms-utils.h"

#define KMS_RTMP_SENDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTMP_SENDER, KmsRtmpSenderPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTMP_SENDER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTMP_SENDER(obj)->priv->mutex))

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

	KmsRtmpSession *neg_spec;
	gboolean offerer;
	guint timeout;

	gchar *url;
	GstElement *audio_elem;
	GstElement *video_elem;
};

enum {
	PROP_0,

	PROP_NEG_SPEC,
	PROP_OFFERER,
};

G_DEFINE_TYPE(KmsRtmpSender, kms_rtmp_sender, KMS_TYPE_MEDIA_HANDLER_SINK)

G_LOCK_DEFINE_STATIC(rtmp_sender_lock);

static void
dispose_neg_spec(KmsRtmpSender *self) {
	if (self->priv->neg_spec != NULL) {
		g_object_unref(self->priv->neg_spec);
		self->priv->neg_spec = NULL;
	}
}

static void
dispose_audio_elem(KmsRtmpSender *self) {
	if (self->priv->audio_elem != NULL) {
		g_object_unref(self->priv->audio_elem);
		self->priv->audio_elem = NULL;
	}
}

static void
dispose_video_elem(KmsRtmpSender *self) {
	if (self->priv->video_elem != NULL) {
		g_object_unref(self->priv->video_elem);
		self->priv->video_elem = NULL;
	}
}

static void
dispose_timeout(KmsRtmpSender *self) {
	if (self->priv->timeout != 0) {
		g_source_remove(self->priv->timeout);
		self->priv->timeout = 0;
	}
}

static void
remove_rtmpsink(KmsRtmpSender *self) {
	GstElement *rtmpsink, *queue, *flvmux;

	rtmpsink = gst_bin_get_by_name(GST_BIN(self), "rtmpsink");
	queue = gst_bin_get_by_name(GST_BIN(self), "queue");
	flvmux = gst_bin_get_by_name(GST_BIN(self), "flvmux");

	if (flvmux != NULL) {
		gst_bin_remove(GST_BIN(self), flvmux);
		gst_element_set_state(flvmux, GST_STATE_NULL);
		gst_object_unref(flvmux);
	}

	if (queue != NULL) {
		gst_bin_remove(GST_BIN(self), queue);
		gst_element_set_state(queue, GST_STATE_NULL);
		gst_object_unref(queue);
	}

	if (rtmpsink != NULL) {
		gst_bin_remove(GST_BIN(self), rtmpsink);
		G_LOCK(rtmp_sender_lock);
		gst_element_set_state(rtmpsink, GST_STATE_NULL);
		G_UNLOCK(rtmp_sender_lock);
		gst_object_unref(rtmpsink);
	}
}

static gboolean
create_rtmpsink_cb(KmsRtmpSender *self) {
	GstElement *rtmpsink, *queue, *flvmux;

	LOCK(self);
	if (self->priv->timeout == 0)
		goto end;

	self->priv->timeout = 0;

	remove_rtmpsink(self);

	rtmpsink = gst_element_factory_make("rtmpsink", "rtmpsink");
	queue = kms_utils_create_queue("queue");
	flvmux = gst_element_factory_make("flvmux", "flvmux");

	if (queue == NULL || rtmpsink == NULL || flvmux == NULL) {
		if (queue != NULL)
			gst_object_unref(queue);

		if (rtmpsink != NULL)
			gst_object_unref(rtmpsink);

		if (flvmux != NULL)
			gst_object_unref(flvmux);

		goto end;
	}

	g_object_set(flvmux, "streamable", TRUE, NULL);
	g_object_set(rtmpsink, "sync", FALSE, NULL);
	g_object_set(rtmpsink, "async", FALSE, NULL);
	g_object_set(rtmpsink, "enable-last-buffer", FALSE, NULL);
	g_object_set(rtmpsink, "blocksize", 10, NULL);
	g_object_set(rtmpsink, "location", self->priv->url, NULL);

	G_LOCK(rtmp_sender_lock);
	gst_element_set_state(rtmpsink, GST_STATE_PLAYING);
	G_UNLOCK(rtmp_sender_lock);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(flvmux, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), rtmpsink, queue, flvmux, NULL);
	gst_element_link_many(flvmux, queue, rtmpsink, NULL);

	if (self->priv->audio_elem != NULL)
		gst_element_link(self->priv->audio_elem, flvmux);

	if (self->priv->video_elem != NULL)
		gst_element_link(self->priv->video_elem, flvmux);

end:
	UNLOCK(self);
	return FALSE;
}

static void
create_rtmpsink(KmsRtmpSender *self) {
	/* Self should be locked */

	if (self->priv->timeout != 0)
		/* Callback already in process */
		return;

	self->priv->timeout = g_timeout_add(500,
					(GSourceFunc) create_rtmpsink_cb,
					self);
}

static void
set_video_elem(KmsRtmpSender *self, GstElement *video_elem) {
	LOCK(self);
	dispose_video_elem(self);
	self->priv->video_elem = gst_object_ref(video_elem);
	create_rtmpsink(self);
	UNLOCK(self);
}

static void
set_audio_elem(KmsRtmpSender *self, GstElement *audio_elem) {
	LOCK(self);
	dispose_audio_elem(self);
	self->priv->audio_elem = gst_object_ref(audio_elem);
	create_rtmpsink(self);
	UNLOCK(self);
}

static void
unset_video_elem(KmsRtmpSender *self) {
	LOCK(self);
	dispose_video_elem(self);
	create_rtmpsink(self);
	UNLOCK(self);
}

static void
unset_audio_elem(KmsRtmpSender *self) {
	LOCK(self);
	dispose_audio_elem(self);
	create_rtmpsink(self);
	UNLOCK(self);
}

static void
audio_unlinked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	unset_audio_elem(self);
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);
}

static void
video_unlinked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	unset_video_elem(self);
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);
}

static void
found_audio(GstElement *tf, guint prob, GstCaps *caps, KmsRtmpSender *self) {
	GstElement *queue;
	GstPad *sink;

	queue = kms_utils_create_queue(NULL);

	sink = gst_element_get_static_pad(queue, "sink");
	kms_utils_remove_when_unlinked(sink);
	gst_object_unref(sink);

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), queue);

	gst_element_link(tf, queue);

	set_audio_elem(self, queue);
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
	kms_utils_remove_when_unlinked(sink);

	g_object_unref(sink);
}

static void
found_video(GstElement *tf, guint prob, GstCaps *caps, KmsRtmpSender *self) {
	GstElement *queue;
	GstPad *sink;

	queue = kms_utils_create_queue(NULL);

	sink = gst_element_get_static_pad(queue, "sink");
	kms_utils_remove_when_unlinked(sink);
	gst_object_unref(sink);

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), queue);

	gst_element_link(tf, queue);

	set_video_elem(self, queue);
}

static void
video_linked(GstPad *pad, GstPad *peer, KmsRtmpSender *self) {
	GstElement *typefind;
	GstPad *sink;

	typefind = gst_element_factory_make("typefind", NULL);
	if (typefind == NULL)
		return;

	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), typefind);

	g_object_connect(typefind, "signal::have-type", found_video, self, NULL);

	sink = gst_element_get_static_pad(typefind, "sink");
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), sink);
	kms_utils_remove_when_unlinked(sink);

	g_object_unref(sink);
}

static void
create_media_chain(KmsRtmpSender *self) {
	GstPadTemplate *audio_templ, *video_templ;
	GstPad *audio_pad, *video_pad;
	gchar *url;

	url = kms_rtmp_session_get_url(self->priv->neg_spec,
							self->priv->offerer);
	self->priv->url = url;

	if (url == NULL) {
		g_warn_if_reached();
		return;
	}

	/* TODO: Get caps from flvmux factory */
	audio_templ = gst_pad_template_new("audio_sink",
					GST_PAD_SINK,
					GST_PAD_ALWAYS,
					gst_caps_from_string(AUDIO_CAPS));
	audio_pad = gst_ghost_pad_new_no_target_from_template("audio_sink",
								audio_templ);
	if (audio_pad != NULL) {
		gst_pad_set_active(audio_pad, TRUE);
		g_object_connect(audio_pad, "signal::unlinked", audio_unlinked,
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
		g_object_connect(video_pad, "signal::unlinked", video_unlinked,
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
	dispose_timeout(self);
	dispose_neg_spec(self);
	dispose_audio_elem(self);
	dispose_video_elem(self);
	UNLOCK(self);

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	g_mutex_free(self->priv->mutex);
	if (self->priv->url != NULL)
		g_free(self->priv->url);

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
	self->priv->url = NULL;
	self->priv->audio_elem = NULL;
	self->priv->video_elem = NULL;
	self->priv->timeout = 0;
}
