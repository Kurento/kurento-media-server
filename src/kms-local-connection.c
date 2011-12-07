#include "kms-local-connection.h"

#define KMS_LOCAL_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_LOCAL_CONNECTION, KmsLocalConnectionPriv))

struct _KmsLocalConnectionPriv {
	GString *name;
};

G_DEFINE_TYPE(KmsLocalConnection, kms_local_connection, KMS_TYPE_CONNECTION)

static void
kms_local_connection_class_init (KmsLocalConnectionClass *klass) {
	g_type_class_add_private (klass, sizeof (KmsLocalConnectionPriv));
}

static void
kms_local_connection_init (KmsLocalConnection *self) {
	self->priv = KMS_LOCAL_CONNECTION_GET_PRIVATE(self);
}
