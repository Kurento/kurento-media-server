#include "endpoint.h"

#define KMS_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_ENDPOINT, KmsEndpointPriv))

struct _KmsEndpointPriv {
	GString *name;
};

static void
kms_endpoint_class_init (KmsEndpointClass *klass) {
	g_type_class_add_private (klass, sizeof (KmsEndpointPriv));
}

static void
kms_endpoint_init (KmsEndpoint *self) {
	self->priv = KMS_ENDPOINT_GET_PRIVATE (self);
}
