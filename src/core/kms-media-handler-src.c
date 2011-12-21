#include <kms-core.h>

#define KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))

struct _KmsMediaHandlerSrcPriv {
	GStaticMutex mutex;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsMediaHandlerSrc, kms_media_handler_src, G_TYPE_OBJECT)

static void
kms_media_handler_src_class_init(KmsMediaHandlerSrcClass *klass) {
	g_type_class_add_private(klass, sizeof(KmsMediaHandlerSrcPriv));
}

static void
kms_media_handler_src_init(KmsMediaHandlerSrc *self) {
	self->priv = KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(self);
}
