#include <kms-core.h>

#define KMS_MEDIA_HANDLER_SINK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSinkPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_MEDIA_HANDLER_SINK(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_MEDIA_HANDLER_SINK(obj)->priv->mutex)))

struct _KmsMediaHandlerSinkPriv {
	GStaticMutex mutex;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsMediaHandlerSink, kms_media_handler_sink, G_TYPE_OBJECT)

static void
kms_media_handler_sink_class_init(KmsMediaHandlerSinkClass *klass) {
	g_type_class_add_private(klass, sizeof(KmsMediaHandlerSinkPriv));
}

static void
kms_media_handler_sink_init(KmsMediaHandlerSink *self) {
	self->priv = KMS_MEDIA_HANDLER_SINK_GET_PRIVATE(self);
}
