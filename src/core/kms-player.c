#include <kms-core.h>

#define KMS_PLAYER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_CONNECTION, KmsPlayerPriv))

#define LOCK(obj) (g_mutex_lock(KMS_PLAYER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_PLAYER(obj)->priv->mutex))

struct _KmsPlayerPriv {
	GMutex *mutex;
};

G_DEFINE_TYPE(KmsPlayer, kms_player, KMS_TYPE_RESOURCE)

void
kms_player_set_url(KmsPlayer *self, gchar *url) {
	KMS_PLAYER_GET_CLASS(self)->set_url(self, url);
}

void
kms_player_start(KmsPlayer *self) {
	KMS_PLAYER_GET_CLASS(self)->start(self);
}

void
kms_player_stop(KmsPlayer *self) {
	KMS_PLAYER_GET_CLASS(self)->stop(self);
}

void
kms_player_set_url_default(KmsPlayer *self, gchar *url) {
	g_warning("set_url method not implemented in %s class",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
}

void
kms_player_start_default(KmsPlayer *self) {
	g_warning("start method not implemented in %s class",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
}

void
kms_player_stop_default(KmsPlayer *self) {
	g_warning("stop method not implemented in %s class",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
}

static void
kms_player_dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_player_parent_class)->dispose(object);
}

static void
kms_player_finalize(GObject *object) {
	KmsPlayer *self = KMS_PLAYER(object);

	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_player_parent_class)->finalize(object);
}

static void
kms_player_class_init(KmsPlayerClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private(klass, sizeof (KmsPlayerPriv));

	gobject_class->dispose = kms_player_dispose;
	gobject_class->finalize = kms_player_finalize;

	klass->start = kms_player_start_default;
	klass->set_url = kms_player_set_url_default;
	klass->stop = kms_player_stop_default;
}

static void
kms_player_init(KmsPlayer *self) {
	self->priv = KMS_PLAYER_GET_PRIVATE (self);

	self->priv->mutex = g_mutex_new();
}
