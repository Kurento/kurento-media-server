#include "kms-connection.h"

#define KMS_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_CONNECTION, KmsConnectionPriv))

struct _KmsConnectionPriv {
	GString *name;
};

G_DEFINE_TYPE(KmsConnection, kms_connection, G_TYPE_OBJECT);

static void
kms_connection_class_init (KmsConnectionClass *klass) {
	g_type_class_add_private (klass, sizeof (KmsConnectionPriv));
}

static void
kms_connection_init (KmsConnection *self) {
	self->priv = KMS_CONNECTION_GET_PRIVATE (self);
}
