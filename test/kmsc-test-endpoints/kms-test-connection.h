#ifndef __KMS_TEST_CONNECTION_H__
#define __KMS_TEST_CONNECTION_H__

#include <glib-object.h>
#include "kms-connection.h"

/*
 * Type macros.
 */
#define KMS_TYPE_TEST_CONNECTION		(kms_test_connection_get_type ())
#define KMS_TEST_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_TEST_CONNECTION, KmsTestConnection))
#define KMS_IS_TEST_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_TEST_CONNECTION))
#define KMS_TEST_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_TEST_CONNECTION, KmsTestConnectionClass))
#define KMS_IS_TEST_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_TEST_CONNECTION))
#define KMS_TEST_CONNECTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_TEST_CONNECTION, KmsTestConnectionClass))

typedef struct _KmsTestConnection		KmsTestConnection;
typedef struct _KmsTestConnectionClass		KmsTestConnectionClass;
typedef struct _KmsTestConnectionPriv		KmsTestConnectionPriv;

struct _KmsTestConnection {
	KmsConnection parent_instance;

	/* instance members */

	/*
	KmsTestConnectionPriv *priv;
	*/
};

struct _KmsTestConnectionClass {
	KmsConnectionClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_TEST_CONNECTION */
GType kms_test_connection_get_type (void);

/*
 * Method definitions.
 */

#endif /* __KMS_TEST_CONNECTION_H__ */
