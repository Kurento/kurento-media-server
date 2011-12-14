#include "kms-rtp-endpoint.h"
#include "kms-rtp-connection.h"

#define KMS_RTP_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTP_ENDPOINT, KmsRtpEndpointPriv))

/*
struct _KmsRtpEndpointPriv {
};
*/

G_DEFINE_TYPE(KmsRtpEndpoint, kms_rtp_endpoint, KMS_TYPE_ENDPOINT)

static KmsConnection*
create_connection(KmsEndpoint *self, gchar *name, GError **err) {
	KmsConnection *conn;

	conn = g_object_new(KMS_TYPE_RTP_CONNECTION, "id", name,
						"endpoint", self, NULL);

	g_object_set(G_OBJECT(self), "manager", conn, NULL);
	return conn;
}

static void
kms_rtp_endpoint_class_init (KmsRtpEndpointClass *klass) {
	/* g_type_class_add_private (klass, sizeof (KmsRtpEndpointPriv)); */
	KMS_ENDPOINT_CLASS(klass)->create_connection = create_connection;
}

static void
kms_rtp_endpoint_init (KmsRtpEndpoint *self) {
	/*
	self->priv = KMS_RTP_ENDPOINT_GET_PRIVATE(self);
	*/
}
