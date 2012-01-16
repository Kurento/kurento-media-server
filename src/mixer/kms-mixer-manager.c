#include <mixer/kms-mixer-manager.h>

#define KMS_MIXER_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_MIXER_MANAGER, KmsMixerManagerPriv))

#define LOCK(obj) (g_mutex_lock(KMS_MIXER_MANAGER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_MIXER_MANAGER(obj)->priv->mutex))

struct _KmsMixerManagerPriv {
	GMutex *mutex;
};

enum {
	PROP_0,
};

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsMixerManager, kms_mixer_manager, G_TYPE_OBJECT,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init))

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *manager) {
	KMS_DEBUG;
	g_print("TODO: Implement this function >%s<\n", __FUNCTION__);
	return NULL;
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
}

static void
constructed(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_manager_parent_class)->constructed(object);
}

static void
dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_manager_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerManager *self = KMS_MIXER_MANAGER(object);

	if (self->priv->mutex != NULL) {
		g_mutex_free(self->priv->mutex);
		self->priv->mutex = NULL;
	}

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_manager_parent_class)->finalize(object);
}

static void
kms_mixer_manager_class_init(KmsMixerManagerClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsMixerManagerPriv));

	gobject_class->constructed = constructed;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
}

static void
kms_mixer_manager_init(KmsMixerManager *self) {
	self->priv = KMS_MIXER_MANAGER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
