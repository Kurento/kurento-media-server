#include <player/kms-player-endpoint.h>

#define KMS_PLAYER_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_PLAYER_ENDPOINT, KmsPlayerEndpointPriv))

#define LOCK(obj) (g_mutex_lock(KMS_PLAYER_ENDPOINT(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_PLAYER_ENDPOINT(obj)->priv->mutex))

struct _KmsPlayerEndpointPriv {
	GMutex *mutex;
};

enum {
	PROP_0,
};

G_DEFINE_TYPE(KmsPlayerEndpoint, kms_player_endpoint, KMS_TYPE_ENDPOINT)

static void
dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_player_endpoint_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsPlayerEndpoint *self = KMS_PLAYER_ENDPOINT(object);

	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_player_endpoint_parent_class)->finalize(object);
}

static void
kms_player_endpoint_class_init(KmsPlayerEndpointClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsPlayerEndpointPriv));

	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
}

static void
kms_player_endpoint_init(KmsPlayerEndpoint *self) {
	self->priv = KMS_PLAYER_ENDPOINT_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
