#include <mixer/kms-mixer.h>

#define KMS_MIXER_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_MIXER_ENDPOINT, KmsMixerEndpointPriv))

#define LOCK(obj) (g_mutex_lock(KMS_MIXER_ENDPOINT(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_MIXER_ENDPOINT(obj)->priv->mutex))

struct _KmsMixerEndpointPriv {
	GMutex *mutex;
};

enum {
	PROP_0,
};

G_DEFINE_TYPE(KmsMixerEndpoint, kms_mixer_endpoint, KMS_TYPE_ENDPOINT)

static void
dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_endpoint_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerEndpoint *self = KMS_MIXER_ENDPOINT(object);

	if (self->priv->mutex != NULL) {
		g_mutex_free(self->priv->mutex);
		self->priv->mutex = NULL;
	}

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_endpoint_parent_class)->finalize(object);
}

static void
kms_mixer_endpoint_class_init(KmsMixerEndpointClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsMixerEndpointPriv));

	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
}

static void
kms_mixer_endpoint_init(KmsMixerEndpoint *self) {
	self->priv = KMS_MIXER_ENDPOINT_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
