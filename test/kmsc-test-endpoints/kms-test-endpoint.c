#include "kms-test-endpoint.h"
#include "kms-test-connection.h"

#define KMS_TEST_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_TEST_ENDPOINT, KmsTestEndpointPriv))

/*
struct _KmsTestEndpointPriv {
};
*/

G_DEFINE_TYPE(KmsTestEndpoint, kms_test_endpoint, KMS_TYPE_ENDPOINT)

static KmsConnection*
create_connection(KmsEndpoint *self, gchar *name, GError **err) {
	KmsConnection *conn;

	conn = g_object_new(KMS_TYPE_TEST_CONNECTION, "id", name,
						"endpoint", self, NULL);

	g_object_set(G_OBJECT(self), "manager", conn, NULL);
	return conn;
}

static void
kms_test_endpoint_class_init (KmsTestEndpointClass *klass) {
	/* g_type_class_add_private (klass, sizeof (KmsTestEndpointPriv)); */
	KMS_ENDPOINT_CLASS(klass)->create_connection = create_connection;
}

static void
kms_test_endpoint_init (KmsTestEndpoint *self) {
	/*
	self->priv = KMS_TEST_ENDPOINT_GET_PRIVATE(self);
	*/
}
