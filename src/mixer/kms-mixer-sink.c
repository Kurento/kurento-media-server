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

G_DEFINE_TYPE(KmsMixerSink, kms_mixer_sink, KMS_TYPE_MEDIA_HANDLER_SINK)

static void
constructed(GObject *object) {
	G_OBJECT_CLASS(kms_mixer_sink_parent_class)->constructed(object);
}

static void
dispose(GObject *object) {
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

	g_type_class_add_private(klass, sizeof(KmsMixerSinkPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->constructed = constructed;

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_mixer_sink_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_mixer_sink_parent_class)->numpadtemplates;
}

static void
kms_mixer_sink_init(KmsMixerSink *self) {
	self->priv = KMS_MIXER_SINK_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
