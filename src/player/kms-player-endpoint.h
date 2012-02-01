#ifndef __KMS_PLAYER_ENDPOINT_H__
#define __KMS_PLAYER_ENDPOINT_H__

#include <glib-object.h>
#include <kms-core.h>

/*
 * Type macros.
 */
#define KMS_TYPE_PLAYER_ENDPOINT		(kms_player_endpoint_get_type ())
#define KMS_PLAYER_ENDPOINT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_PLAYER_ENDPOINT, KmsPlayerEndpoint))
#define KMS_IS_PLAYER_ENDPOINT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_PLAYER_ENDPOINT))
#define KMS_PLAYER_ENDPOINT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_PLAYER_ENDPOINT, KmsPlayerEndpointClass))
#define KMS_IS_PLAYER_ENDPOINT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_PLAYER_ENDPOINT))
#define KMS_PLAYER_ENDPOINT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_PLAYER_ENDPOINT, KmsPlayerEndpointClass))

#define KMS_PLAYER_ENDPOINT_ERROR (g_quark_from_string("KmsPlayerEndpointError"))

typedef struct _KmsPlayerEndpoint	KmsPlayerEndpoint;
typedef struct _KmsPlayerEndpointClass	KmsPlayerEndpointClass;
typedef struct _KmsPlayerEndpointPriv	KmsPlayerEndpointPriv;

struct _KmsPlayerEndpoint {
	KmsEndpoint parent_instance;

	/* instance members */

	KmsPlayerEndpointPriv *priv;
};

struct _KmsPlayerEndpointClass {
	KmsEndpointClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_PLAYER_ENDPOINT */
GType kms_player_endpoint_get_type (void);

/*
 * Method definitions.
 */

#endif /* __KMS_PLAYER_ENDPOINT_H__ */
