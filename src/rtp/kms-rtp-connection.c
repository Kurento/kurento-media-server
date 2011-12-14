#include "kms-rtp-connection.h"
#include "kms-media-handler-manager.h"

#define KMS_RTP_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTP_CONNECTION, KmsRtpConnectionPriv))

/*
struct _KmsRtpConnectionPriv {
};
*/

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsRtpConnection, kms_rtp_connection,
				KMS_TYPE_CONNECTION,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init))

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	/*KmsRtpConnection *self = KMS_RTP_CONNECTION(iface);*/

	g_print("No media handler factory defined yet");
	return NULL;
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
}

static void
kms_rtp_connection_class_init (KmsRtpConnectionClass *klass) {
	/*
	g_type_class_add_private (klass, sizeof (KmsRtpConnectionPriv));
	*/
}

static void
kms_rtp_connection_init (KmsRtpConnection *self) {
	/*
	self->priv = KMS_RTP_CONNECTION_GET_PRIVATE(self);
	*/
}
