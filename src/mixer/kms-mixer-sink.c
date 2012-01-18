#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"
#include <mixer/kms-mixer-sink.h>

#define KMS_MIXER_SINK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MIXER_SINK, KmsMixerSinkPriv))

#define LOCK(obj) (g_mutex_lock(KMS_MIXER_SINK(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_MIXER_SINK(obj)->priv->mutex))

struct _KmsMixerSinkPriv {
	GMutex *mutex;
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

void
kms_mixer_sink_link(KmsMixerSink *sink, KmsMixerSrc *src) {
	g_return_if_fail(KMS_IS_MIXER_SINK(sink) && KMS_IS_MIXER_SRC(src));
	KMS_LOG_DEBUG("TODO: Implement this method");
}

static GstPadLinkReturn
link_pad(GstPad *pad, GstPad *peer) {
	/* TODO: Implement this function */
	KMS_LOG_DEBUG("TODO: Implement link_pad");
	return GST_PAD_LINK_OK;
}

static void
pad_unlinked(GstPad  *pad, GstPad  *peer, GstElement *elem) {
	/* TODO: Implement this function */
	KMS_LOG_DEBUG("TODO: Implement pad_unlinked");
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

	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(sink, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), tee, queue, sink, NULL);

	gst_element_link_many(tee, queue, sink, NULL);

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

	kms_utils_remove_src_pads(GST_ELEMENT(object));

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
