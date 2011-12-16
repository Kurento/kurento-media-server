#include "kms-test-connection.h"
#include <kms-core.h>

#define KMS_TEST_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_TEST_CONNECTION, KmsTestConnectionPriv))

/*
struct _KmsTestConnectionPriv {
};
*/

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsTestConnection, kms_test_connection,
				KMS_TYPE_CONNECTION,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init))

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	/*KmsTestConnection *self = KMS_TEST_CONNECTION(iface);*/

	g_print("No media handler factory defined yet");
	return NULL;
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
}

static void
kms_test_connection_class_init (KmsTestConnectionClass *klass) {
	/*
	g_type_class_add_private (klass, sizeof (KmsTestConnectionPriv));
	*/
}

static void
kms_test_connection_init (KmsTestConnection *self) {
	/*
	self->priv = KMS_TEST_CONNECTION_GET_PRIVATE(self);
	*/
}
