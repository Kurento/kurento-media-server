#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"
#include <mixer/kms-mixer-src.h>

#define KMS_MIXER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MIXER_SRC, KmsMixerSrcPriv))

#define LOCK(obj) (g_mutex_lock(KMS_MIXER_SRC(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_MIXER_SRC(obj)->priv->mutex))

#define MEDIA_TYPE_DATA "type"

struct _KmsMixerSrcPriv {
	GMutex *mutex;

	GstElement *adder;
};

enum {
	PROP_0,
};

static GstStaticPadTemplate audio_sink = GST_STATIC_PAD_TEMPLATE (
				"mixer_audio_sink%d",
				GST_PAD_SINK,
				GST_PAD_REQUEST,
				GST_STATIC_CAPS(
					"audio/x-raw-int, endianness=1234, "
					"signed=true, width=16, depth=16, "
					"rate=8000, channels=1")
);

G_DEFINE_TYPE(KmsMixerSrc, kms_mixer_src, KMS_TYPE_MEDIA_HANDLER_SRC)

static void
dispose_adder(KmsMixerSrc *self) {
	if (self->priv->adder == NULL) {
		g_object_unref(self->priv->adder);
		self->priv->adder = NULL;
	}
}

static GstPadLinkReturn
link_pad(GstPad *pad, GstPad *peer) {
	/* TODO: Implement this function */
	KMS_LOG_DEBUG("TODO: Implement this link_pad\n");
	return GST_PAD_LINK_OK;
}

static void
pad_unlinked(GstPad  *pad, GstPad  *peer, GstElement *elem) {
	/* TODO: Implement this function */
	KMS_LOG_DEBUG("TODO: Implement this pad_unlinked\n");
}

static GstPad*
mixer_src_request_new_pad(GstElement *elem, GstPadTemplate *templ,
							const gchar *name) {
	GstPad *pad;
	gchar *new_name;

	if (g_strcmp0(GST_PAD_TEMPLATE_NAME_TEMPLATE(templ),
					audio_sink.name_template) != 0) {
		GstElementClass *p_class =
				GST_ELEMENT_CLASS(kms_mixer_src_parent_class);
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
create_audio_src(KmsMixerSrc *self) {
	GstElement *adder, *tee, *queue, *fake;
	GstPad *pad;

	adder = gst_element_factory_make("adder", NULL);
	tee = gst_element_factory_make("tee", NULL);
	queue = gst_element_factory_make("queue2", NULL);
	fake = gst_element_factory_make("fakesink", NULL);

	if (adder == NULL || tee == NULL || queue == NULL || fake == NULL) {
		if (adder != NULL)
			g_object_unref(adder);

		if (tee != NULL)
			g_object_unref(tee);

		if (queue != NULL)
			g_object_unref(queue);

		if (fake != NULL)
			g_object_unref(fake);
	}

	kms_utils_release_unlinked_pads(tee);
	kms_utils_release_unlinked_pads(adder);

	self->priv->adder = g_object_ref(adder);

	gst_element_set_state(adder, GST_STATE_PLAYING);
	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(fake, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), adder, tee, queue, fake, NULL);
	gst_element_link_many(adder, tee, queue, fake, NULL);

	pad = gst_element_get_pad(adder, "src");
	kms_media_handler_src_set_raw_pad(KMS_MEDIA_HANDLER_SRC(self), pad, tee,
							KMS_MEDIA_TYPE_AUDIO);
}

static void
constructed(GObject *object) {
	KmsMixerSrc *self = KMS_MIXER_SRC(object);
	G_OBJECT_CLASS(kms_mixer_src_parent_class)->constructed(object);

	create_audio_src(self);
}

static void
dispose(GObject *object) {
	KmsMixerSrc *self = KMS_MIXER_SRC(object);

	kms_utils_remove_sink_pads(GST_ELEMENT(object));
	LOCK(self);
	dispose_adder(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_src_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerSrc *self = KMS_MIXER_SRC(object);

	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_src_parent_class)->finalize(object);
}

static void
kms_mixer_src_class_init(KmsMixerSrcClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GstPadTemplate *templ;

	g_type_class_add_private(klass, sizeof(KmsMixerSrcPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->constructed = constructed;

	GST_ELEMENT_CLASS(klass)->request_new_pad = mixer_src_request_new_pad;

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_mixer_src_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_mixer_src_parent_class)->numpadtemplates;

	templ = gst_static_pad_template_get(&audio_sink);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);
}

static void
kms_mixer_src_init(KmsMixerSrc *self) {
	self->priv = KMS_MIXER_SRC_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
