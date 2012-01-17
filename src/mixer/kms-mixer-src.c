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
constructed(GObject *object) {
	G_OBJECT_CLASS(kms_mixer_src_parent_class)->constructed(object);
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
