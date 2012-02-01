#include <kms-core.h>

#define KMS_RESOURCE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_CONNECTION, KmsResourcePriv))

#define LOCK(obj) (g_mutex_lock(KMS_RESOURCE(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RESOURCE(obj)->priv->mutex))

struct _KmsResourcePriv {
	GMutex *mutex;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsResource, kms_resource, G_TYPE_OBJECT)

static void
kms_resource_dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_resource_parent_class)->dispose(object);
}

static void
kms_resource_finalize(GObject *object) {
	KmsResource *self = KMS_RESOURCE(object);

	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_resource_parent_class)->finalize(object);
}

static void
kms_resource_class_init(KmsResourceClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private(klass, sizeof (KmsResourcePriv));

	gobject_class->dispose = kms_resource_dispose;
	gobject_class->finalize = kms_resource_finalize;
}

static void
kms_resource_init(KmsResource *self) {
	self->priv = KMS_RESOURCE_GET_PRIVATE (self);

	self->priv->mutex = g_mutex_new();
}
