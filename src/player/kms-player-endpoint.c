#include <kms-core.h>
#include <player/kms-player-endpoint.h>
#include <player/kms-player-src.h>

#define KMS_PLAYER_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_PLAYER_ENDPOINT, KmsPlayerEndpointPriv))

#define LOCK(obj) (g_mutex_lock(KMS_PLAYER_ENDPOINT(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_PLAYER_ENDPOINT(obj)->priv->mutex))

struct _KmsPlayerEndpointPriv {
	GMutex *mutex;

	KmsPlayerSrc *player;
};

enum {
	PROP_0,
};

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);
static void media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsPlayerEndpoint, kms_player_endpoint,
				KMS_TYPE_ENDPOINT,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init)
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_FACTORY,
					media_handler_factory_iface_init)
				)

static void
dispose_player(KmsPlayerEndpoint *self) {
	if (self->priv->player != NULL) {
		g_object_unref(self->priv->player);
		self->priv->player = NULL;
	}
}

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	return KMS_MEDIA_HANDLER_FACTORY(g_object_ref(iface));
}

static void
dispose_factory(KmsMediaHandlerManager *manager,  KmsMediaHandlerFactory *factory) {
	/* Nothing to do, just avoid the warning */
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
	iface->dispose_factory = dispose_factory;
}

static KmsMediaHandlerSrc*
get_src(KmsMediaHandlerFactory *iface) {
	KmsPlayerEndpoint *self;
	KmsMediaHandlerSrc *src;

	if (!KMS_IS_PLAYER_ENDPOINT(iface))
		return NULL;

	self = KMS_PLAYER_ENDPOINT(iface);

	LOCK(self);
	if (self->priv->player != NULL)
		src = KMS_MEDIA_HANDLER_SRC(g_object_ref(self->priv->player));
	else
		src = NULL;
	UNLOCK(self);

	return src;
}

static KmsMediaHandlerSink*
get_sink(KmsMediaHandlerFactory *iface) {
	return NULL;
}

static void
media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface) {
	iface->get_sink = get_sink;
	iface->get_src = get_src;
}

static KmsResource*
get_resource(KmsEndpoint *endpoint, GType type) {
	KmsPlayerEndpoint *self;
	KmsResource *resource;

	if (!KMS_IS_PLAYER_ENDPOINT(endpoint))
		return NULL;

	self = KMS_PLAYER_ENDPOINT(endpoint);

	LOCK(self);
	if (G_TYPE_CHECK_INSTANCE_TYPE(self->priv->player, type)) {
		resource = KMS_RESOURCE(g_object_ref(self->priv->player));
	} else {
		resource = NULL;
	}
	UNLOCK(self);

	return resource;
}

static void
constructed(GObject *object) {
	KmsPlayerEndpoint *self = KMS_PLAYER_ENDPOINT(object);

	self->priv->player = g_object_new(KMS_TYPE_PLAYER_SRC, NULL);
}

static void
dispose(GObject *object) {
	KmsPlayerEndpoint *self = KMS_PLAYER_ENDPOINT(object);

	LOCK(self);
	dispose_player(self);
	UNLOCK(self);

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
	gobject_class->constructed = constructed;
	KMS_ENDPOINT_CLASS(klass)->get_resource = get_resource;
}

static void
kms_player_endpoint_init(KmsPlayerEndpoint *self) {
	self->priv = KMS_PLAYER_ENDPOINT_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->player = NULL;
}
