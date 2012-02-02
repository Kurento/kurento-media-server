#include <kms-core.h>
#include <player/kms-player-src.h>
#include "internal/kms-utils.h"

#define KMS_PLAYER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_PLAYER_SRC, KmsPlayerSrcPriv))

#define LOCK(obj) (g_mutex_lock(KMS_PLAYER_SRC(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_PLAYER_SRC(obj)->priv->mutex))

struct _KmsPlayerSrcPriv {
	GMutex *mutex;
};

static void player_iface_init(KmsPlayerInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsPlayerSrc, kms_player_src,
				KMS_TYPE_MEDIA_HANDLER_SRC,
				G_IMPLEMENT_INTERFACE(KMS_TYPE_PLAYER_SRC,
							player_iface_init)
				)

static void
set_url(KmsPlayer *player, gchar *url) {
	KMS_LOG_DEBUG("TODO: Implement set_url");
}

void
start(KmsPlayer *player) {
	KMS_LOG_DEBUG("TODO: Implement start");
}

void
stop(KmsPlayer *player) {
	KMS_LOG_DEBUG("TODO: Implement stop");
}

static void
player_iface_init(KmsPlayerInterface *iface) {
	g_print("TODO: Review start interface init");
	iface->start = start;
	iface->stop = stop;
	iface->set_url = set_url;
}

static void
constructed(GObject *object) {
	G_OBJECT_CLASS(kms_player_src_parent_class)->constructed(object);
}

static void
dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_player_src_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsPlayerSrc *self = KMS_PLAYER_SRC(object);

	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_player_src_parent_class)->finalize(object);
}

static void
kms_player_src_class_init(KmsPlayerSrcClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsPlayerSrcPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->constructed = constructed;

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_player_src_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_player_src_parent_class)->numpadtemplates;
}

static void
kms_player_src_init(KmsPlayerSrc *self) {
	self->priv = KMS_PLAYER_SRC_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
