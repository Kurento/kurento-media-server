#ifndef __KMS_PAYLOAD_H__
#define __KMS_PAYLOAD_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_PAYLOAD		(kms_payload_get_type ())
#define KMS_PAYLOAD(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_PAYLOAD, KmsPayload))
#define KMS_IS_PAYLOAD(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_PAYLOAD))
#define KMS_PAYLOAD_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_PAYLOAD, KmsPayloadClass))
#define KMS_IS_PAYLOAD_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_PAYLOAD))
#define KMS_PAYLOAD_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_PAYLOAD, KmsPayloadClass))

typedef struct _KmsPayload	KmsPayload;
typedef struct _KmsPayloadClass	KmsPayloadClass;
typedef struct _KmsPayloadPriv	KmsPayloadPriv;

#include "kms-endpoint.h"

struct _KmsPayload {
	GObject parent_instance;

	/* instance members */

	KmsPayloadPriv *priv;
};

struct _KmsPayloadClass {
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_PAYLOAD */
GType kms_payload_get_type (void);

#endif /* __KMS_PAYLOAD_H__ */
