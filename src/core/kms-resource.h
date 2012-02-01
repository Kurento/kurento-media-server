#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_RESOURCE_H__
#define __KMS_RESOURCE_H__

#include <glib-object.h>
#include "kms-enums.h"
#include "kms-sdp-session.h"

/*
 * Type macros.
 */
#define KMS_TYPE_RESOURCE		(kms_resource_get_type())
#define KMS_RESOURCE(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_RESOURCE, KmsResource))
#define KMS_IS_RESOURCE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_RESOURCE))
#define KMS_RESOURCE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_INTERFACE((obj), KMS_TYPE_RESOURCE, KmsResourceClass))

typedef struct _KmsResource		KmsResource;
typedef struct _KmsResourceInterface	KmsResourceInterface;

struct _KmsResourceInterface {
	GTypeInterface parent_class;

	/* class members */
};

/* used by KMS_TYPE_RESOURCE */
GType kms_resource_get_type (void);

/*
 * Method definitions.
 */

#endif /* __KMS_RESOURCE_H__ */
