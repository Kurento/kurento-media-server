#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"
#include <mixer/kms-mixer-sink.h>

#define KMS_MIXER_SINK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MIXER_SINK, KmsMixerSinkPriv))

#define LOCK(obj) (g_mutex_lock(KMS_MIXER_SINK(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_MIXER_SINK(obj)->priv->mutex))

struct _KmsMixerSinkPriv {
	GMutex *mutex;

	GstElement *tee;
};

enum {
	PROP_0,
};

static GstStaticPadTemplate audio_sink = GST_STATIC_PAD_TEMPLATE (
				"audio_sink",
				GST_PAD_SINK,
				GST_PAD_ALWAYS,
				GST_STATIC_CAPS(
					"audio/x-raw-int, endianness=1234, "
					"signed=true, width=16, depth=16, "
					"rate=8000, channels=1")
);

static GstStaticPadTemplate audio_src = GST_STATIC_PAD_TEMPLATE (
				"mixer_audio_src%d",
				GST_PAD_SRC,
				GST_PAD_REQUEST,
				GST_STATIC_CAPS(
					"audio/x-raw-int, endianness=1234, "
					"signed=true, width=16, depth=16, "
					"rate=8000, channels=1")
);

G_DEFINE_TYPE(KmsMixerSink, kms_mixer_sink, KMS_TYPE_MEDIA_HANDLER_SINK)

static void
release_tee(KmsMixerSink *self) {
	if (self->priv->tee != NULL) {
		g_object_unref(self->priv->tee);
		self->priv->tee = NULL;
	}
}

void
kms_mixer_sink_link(KmsMixerSink *sink, KmsMixerSrc *src) {
	GstPad *sink_pad, *src_pad;
	g_return_if_fail(KMS_IS_MIXER_SINK(sink) && KMS_IS_MIXER_SRC(src));

	src_pad = gst_element_get_request_pad(GST_ELEMENT(sink),
							"mixer_audio_src%d");
	sink_pad = gst_element_get_request_pad(GST_ELEMENT(src),
							"mixer_audio_sink%d");

	if (src_pad == NULL || sink_pad == NULL) {
		if (src_pad != NULL) {
			gst_element_release_request_pad(GST_ELEMENT(sink),
								src_pad);
			gst_object_unref(src_pad);
		}

		if (sink_pad != NULL) {
			gst_element_release_request_pad(GST_ELEMENT(src),
								sink_pad);
			gst_object_unref(sink_pad);
		}
	}

	if (GST_PAD_LINK_FAILED(gst_pad_link(src_pad, sink_pad))) {
		gst_element_release_request_pad(GST_ELEMENT(sink), src_pad);
		gst_object_unref(src_pad);
		gst_element_release_request_pad(GST_ELEMENT(src), sink_pad);
		gst_object_unref(sink_pad);
	}
}

static GstPadLinkReturn
add_target_pad(KmsMixerSink *self, GstPad *pad) {
	GstElement *tee, *queue;
	GstPad *target_pad;
	GstPadLinkReturn ret;

	LOCK(self);
	tee = self->priv->tee;
	UNLOCK(self);

	if (tee == NULL)
		return GST_PAD_LINK_WRONG_HIERARCHY;

	queue = kms_utils_create_queue(NULL);

	if (queue == NULL)
		return GST_PAD_LINK_WRONG_HIERARCHY;

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), queue);
	kms_dynamic_connection_full(tee, queue, "src", TRUE);

	target_pad = gst_element_get_static_pad(queue, "src");

	if (!gst_ghost_pad_set_target(GST_GHOST_PAD(pad), target_pad)) {
		gst_element_set_state(queue, GST_STATE_NULL);
		gst_bin_remove(GST_BIN(self), queue);
		ret = GST_PAD_LINK_REFUSED;
	} else {
		ret = GST_PAD_LINK_OK;
	}

	g_object_unref(target_pad);
	return ret;
}

static GstPadLinkReturn
link_pad(GstPad *pad, GstPad *peer) {
	GstElement *elem;
	KmsMixerSink *self;
	GstPadLinkReturn ret;

	elem = gst_pad_get_parent_element(pad);
	if (elem == NULL)
		return GST_PAD_LINK_WRONG_HIERARCHY;

	self = KMS_MIXER_SINK(elem);

	ret = add_target_pad(self, pad);

	if (GST_PAD_LINKFUNC(peer) && GST_PAD_LINK_SUCCESSFUL(ret))
		ret = GST_PAD_LINKFUNC(peer)(peer, pad);

	g_object_unref(elem);
	return ret;
}

static void
pad_unlinked(GstPad  *pad, GstPad  *peer, GstElement *elem) {
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);
	gst_element_release_request_pad(elem, pad);
}

static GstPad*
mixer_sink_request_new_pad(GstElement *elem, GstPadTemplate *templ,
							const gchar *name) {
	GstPad *pad;
	gchar *new_name;

	if (g_strcmp0(GST_PAD_TEMPLATE_NAME_TEMPLATE(templ),
					audio_src.name_template) != 0) {
		GstElementClass *p_class =
				GST_ELEMENT_CLASS(kms_mixer_sink_parent_class);
		return p_class->request_new_pad(elem, templ, name);
	}

	if (name != NULL)
		new_name = g_strdup(name);
	else
		new_name = kms_utils_generate_pad_name(templ->name_template);

	pad = gst_ghost_pad_new_no_target_from_template(new_name, templ);
	g_free(new_name);

	gst_pad_set_active(pad, TRUE);
	gst_pad_set_link_function(pad, link_pad);
	g_object_connect(pad, "signal::unlinked", pad_unlinked, elem, NULL);

	gst_element_add_pad(elem, pad);
	return pad;
}

static void
create_audio_sink(KmsMixerSink *self) {
	GstElement *tee, *queue, *sink;
	GstPadTemplate *templ;
	GstPad *pad, *sink_pad;

	tee = gst_element_factory_make("tee", NULL);
	queue = kms_utils_create_queue(NULL);
	sink = gst_element_factory_make("fakesink", NULL);

	if (tee == NULL || queue == NULL || sink == NULL) {
		g_warn_if_reached();

		if (tee != NULL)
			g_object_unref(tee);

		if (queue != NULL)
			g_object_unref(queue);

		if (sink != NULL)
			g_object_unref(sink);

		return;
	}

	self->priv->tee = g_object_ref(tee);

	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(sink, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), tee, queue, sink, NULL);

	gst_element_link_many(tee, queue, sink, NULL);
	kms_utils_release_unlinked_pads(tee);

	sink_pad = gst_element_get_static_pad(tee, "sink");

	templ = gst_static_pad_template_get(&audio_sink);
	pad = gst_ghost_pad_new_from_template(templ->name_template,
							sink_pad, templ);
	if (pad != NULL) {
		gst_pad_set_active(pad, TRUE);
		gst_element_add_pad(GST_ELEMENT(self), pad);
	}

	g_object_unref(templ);
	g_object_unref(sink_pad);
}

static void
constructed(GObject *object) {
	G_OBJECT_CLASS(kms_mixer_sink_parent_class)->constructed(object);

	create_audio_sink(KMS_MIXER_SINK(object));
}

static void
dispose(GObject *object) {
	KmsMixerSink *self = KMS_MIXER_SINK(object);

	kms_utils_remove_src_pads(GST_ELEMENT(object));
	LOCK(self);
	release_tee(self);
	UNLOCK(self);

	G_OBJECT_CLASS(kms_mixer_sink_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerSink *self = KMS_MIXER_SINK(object);

	g_mutex_free(self->priv->mutex);

	G_OBJECT_CLASS(kms_mixer_sink_parent_class)->finalize(object);
}

static void
kms_mixer_sink_class_init(KmsMixerSinkClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GstPadTemplate *templ;

	g_type_class_add_private(klass, sizeof(KmsMixerSinkPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->constructed = constructed;

	GST_ELEMENT_CLASS(klass)->request_new_pad = mixer_sink_request_new_pad;

	GST_ELEMENT_CLASS(klass)->numpadtemplates = 0;

	templ = gst_static_pad_template_get(&audio_sink);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);

	templ = gst_static_pad_template_get(&audio_src);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);
}

static void
kms_mixer_sink_init(KmsMixerSink *self) {
	self->priv = KMS_MIXER_SINK_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
