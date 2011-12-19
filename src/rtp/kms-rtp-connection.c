#include <kms-core.h>
#include "kms-rtp-connection.h"
#include <gst/gst.h>

#define KMS_RTP_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTP_CONNECTION, KmsRtpConnectionPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTP_CONNECTION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTP_CONNECTION(obj)->priv->mutex)))

struct _KmsRtpConnectionPriv {
	GStaticMutex mutex;
	GstElement *pipe;
};

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);
static void media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsRtpConnection, kms_rtp_connection,
				KMS_TYPE_CONNECTION,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init)
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_FACTORY,
					media_handler_factory_iface_init))

static void
dispose_pipe(KmsRtpConnection *self) {
	if (self->priv->pipe != NULL) {
		g_object_unref(self->priv->pipe);
		self->priv->pipe = NULL;
	}
}

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	return KMS_MEDIA_HANDLER_FACTORY(g_object_ref(iface));
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
}

static KmsMediaHandlerSrc*
get_src(KmsMediaHandlerFactory *self) {
	g_warning("%s:%d Not implemented", __FILE__, __LINE__);
	return NULL;
}

static KmsMediaHandlerSink*
get_sink(KmsMediaHandlerFactory *self) {
	g_warning("%s:%d Not implemented", __FILE__, __LINE__);
	return NULL;
}

static void
media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface) {
	iface->get_sink = get_sink;
	iface->get_src = get_src;
}

static gboolean
connect_to_remote(KmsConnection *self, KmsSdpSession *session, GError **err) {
	KMS_DEBUG;
	g_assert_not_reached();
	return FALSE;
}

static gboolean
mode_changed(KmsConnection *self, KmsConnectionMode mode, KmsMediaType type,
								GError **err) {
	/* TODO: Implement mode changed for RtpConnection */
	return TRUE;
}

static void
constructed(GObject *object) {
	GstElement *pipe;

	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->constructed(object);

	pipe = kms_get_pipeline();

	if (pipe == NULL) {
		g_warning("Kms should be initialized before instantiate "
								"objects");
		g_assert_not_reached();
		return;
	}

	KMS_RTP_CONNECTION(object)->priv->pipe = g_object_ref(pipe);
}

static void
kms_rtp_connection_dispose(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	LOCK(self);
	dispose_pipe(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->dispose(object);
}

static void
kms_rtp_connection_finalize(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->finalize(object);
}

static void
kms_rtp_connection_class_init (KmsRtpConnectionClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private (klass, sizeof (KmsRtpConnectionPriv));

	KMS_CONNECTION_CLASS(klass)->mode_changed = mode_changed;
	KMS_CONNECTION_CLASS(klass)->connect_to_remote = connect_to_remote;
	gobject_class->dispose = kms_rtp_connection_dispose;
	gobject_class->finalize = kms_rtp_connection_finalize;
	gobject_class->constructed = constructed;

}

static void
kms_rtp_connection_init (KmsRtpConnection *self) {
	self->priv = KMS_RTP_CONNECTION_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->pipe = NULL;
}
