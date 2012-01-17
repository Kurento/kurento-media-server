#include <mixer/kms-mixer-factory.h>

#define KMS_MIXER_FACTORY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_MIXER_FACTORY, KmsMixerFactoryPriv))

#define LOCK(obj) (g_mutex_lock(KMS_MIXER_FACTORY(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_MIXER_FACTORY(obj)->priv->mutex))

struct _KmsMixerFactoryPriv {
	GMutex *mutex;
};

enum {
	PROP_0,
};

static void media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsMixerFactory, kms_mixer_factory, G_TYPE_OBJECT,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_FACTORY,
					media_handler_factory_iface_init))

static KmsMediaHandlerSrc*
get_src(KmsMediaHandlerFactory *iface) {
	KMS_LOG_DEBUG("TODO: Implement this get_src");
	return NULL;
}

static KmsMediaHandlerSink*
get_sink(KmsMediaHandlerFactory *iface) {
	KMS_LOG_DEBUG("TODO: Implement this get_sink");
	return NULL;
}

static void
media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface) {
	iface->get_sink = get_sink;
	iface->get_src = get_src;
}

void
kms_mixer_factory_dispose(KmsMixerFactory *self) {
	G_OBJECT_GET_CLASS(self)->dispose(G_OBJECT(self));
}

void
kms_mixer_factory_connect(KmsMixerFactory *self, KmsMixerFactory *other) {
	KMS_LOG_DEBUG("TODO: Implement kms_mixer_factory_connect function");
}

static void
constructed(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_factory_parent_class)->constructed(object);
}

static void
dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_factory_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerFactory *self = KMS_MIXER_FACTORY(object);

	if (self->priv->mutex != NULL) {
		g_mutex_free(self->priv->mutex);
		self->priv->mutex = NULL;
	}

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_factory_parent_class)->finalize(object);
}

static void
kms_mixer_factory_class_init(KmsMixerFactoryClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsMixerFactoryPriv));

	gobject_class->constructed = constructed;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
}

static void
kms_mixer_factory_init(KmsMixerFactory *self) {
	self->priv = KMS_MIXER_FACTORY_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
