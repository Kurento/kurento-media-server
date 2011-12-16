#include <kms-core.h>
#include "kms-rtp-connection.h"

#define KMS_RTP_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTP_CONNECTION, KmsRtpConnectionPriv))

/*
struct _KmsRtpConnectionPriv {
};
*/

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
mode_changed(KmsConnection *self, KmsConnectionMode mode, KmsMediaType type,
								GError **err) {
	/* TODO: Implement mode changed for RtpConnection */
	return TRUE;
}

static void
kms_rtp_connection_class_init (KmsRtpConnectionClass *klass) {
	/*
	g_type_class_add_private (klass, sizeof (KmsRtpConnectionPriv));
	*/

	KMS_CONNECTION_CLASS(klass)->mode_changed = mode_changed;
}

static void
kms_rtp_connection_init (KmsRtpConnection *self) {
	/*
	self->priv = KMS_RTP_CONNECTION_GET_PRIVATE(self);
	*/
}
