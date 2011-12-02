#ifndef __KMS_CONNECTION_H__
#define __KMS_CONNECTION_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_CONNECTION		(kms_connection_get_type ())
#define KMS_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_CONNECTION, KmsConnection))
#define KMS_IS_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_CONNECTION))
#define KMS_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_CONNECTION, KmsConnectionClass))
#define KMS_IS_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_CONNECTION))
#define KMS_CONNECTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_CONNECTION, KmsConnectionClass))

typedef struct _KmsConnection		KmsConnection;
typedef struct _KmsConnectionClass	KmsConnectionClass;
typedef struct _KmsConnectionPriv		KmsConnectionPriv;

struct _KmsConnection
{
	GObject parent_instance;

	/* instance members */

	KmsConnectionPriv *priv;
};

struct _KmsConnectionClass
{
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_CONNECTION */
GType kms_connection_get_type (void);

/*
 * Method definitions.
 */

#endif /* __KMS_CONNECTION_H__ */