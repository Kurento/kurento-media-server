#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_ENDPOINT_H__
#define __KMS_ENDPOINT_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_ENDPOINT		(kms_endpoint_get_type ())
#define KMS_ENDPOINT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_ENDPOINT, KmsEndpoint))
#define KMS_IS_ENDPOINT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_ENDPOINT))
#define KMS_ENDPOINT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_ENDPOINT, KmsEndpointClass))
#define KMS_IS_ENDPOINT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_ENDPOINT))
#define KMS_ENDPOINT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_ENDPOINT, KmsEndpointClass))

#define KMS_ENDPOINT_ERROR (g_quark_from_string("KmsEndpointError"))

enum {
	KMS_ENDPOINT_ERROR_NOT_IMPLEMENTED,
	KMS_ENDPOINT_ERROR_ALREADY_CREATED,
	KMS_ENDPOINT_ERROR_NOT_FOUND
};

typedef struct _KmsEndpoint		KmsEndpoint;
typedef struct _KmsEndpointClass	KmsEndpointClass;
typedef struct _KmsEndpointPriv		KmsEndpointPriv;

#include "kms-connection.h"

struct _KmsEndpoint {
	GObject parent_instance;
	/* instance members */

	KmsEndpointPriv *priv;
};

struct _KmsEndpointClass {
	GObjectClass parent_class;

	/* private methods */

	KmsConnection *(*create_connection) (KmsEndpoint *self, gchar *name,
					     GError **err);

	/* class members */
};

/* used by KMS_TYPE_ENDPOINT */
GType kms_endpoint_get_type (void);

/*
 * Method definitions.
 */

/*
 * Minimal required methods
 *
 * create_connection
 * get_local_name
 * get_state ??
 * delete_connection
 * delete_all_connections
 * get_resource
*/

KmsConnection *kms_endpoint_create_connection(KmsEndpoint *self,
					KmsConnectionType type, GError **err);

/*
 * This will be handled by a property
gchar *kms_endpoint_get_local_name(KmsEndpoint *self);
*/

/*
 * TODO: Implement get_state method.
KmsEndpointState *kms_endpoint_get_state(KmsEndpoint *self);
*/

gboolean kms_endpoint_delete_connection(KmsEndpoint *self, KmsConnection *conn,
								GError **err);

void kms_endpoint_delete_all_connections(KmsEndpoint *self);

/*
 * TODO: Implement get_resource
KmsResource kms_endpoint_get_resource(KmsEndpoint *self, GError *err);
*/

#endif /* __KMS_ENDPOINT_H__ */
