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
};

enum {
	PROP_0,
};

G_DEFINE_TYPE(KmsMixerSrc, kms_mixer_src, KMS_TYPE_MEDIA_HANDLER_SRC)

static void
create_audio_src(KmsMixerSrc *self) {
	GstElement *adder, *tee;
	GstPad *pad;

	adder = gst_element_factory_make("adder", NULL);
	tee = gst_element_factory_make("tee", NULL);

	if (adder == NULL || tee == NULL) {
		if (adder != NULL)
			g_object_unref(adder);

		if (tee != NULL)
			g_object_unref(tee);
	}

	gst_element_set_state(adder, GST_STATE_PLAYING);
	gst_element_set_state(tee, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), adder, tee, NULL);
	gst_element_link_many(adder, tee, NULL);

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

	g_type_class_add_private(klass, sizeof(KmsMixerSrcPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->constructed = constructed;

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_mixer_src_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_mixer_src_parent_class)->numpadtemplates;
}

static void
kms_mixer_src_init(KmsMixerSrc *self) {
	self->priv = KMS_MIXER_SRC_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
