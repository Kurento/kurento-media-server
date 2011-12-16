#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_MEDIA_HANDLER_SRC_H__
#define __KMS_MEDIA_HANDLER_SRC_H__

#include <glib-object.h>

#define KMS_TYPE_MEDIA_HANDLER_SRC			(kms_media_handler_src_get_type())
#define KMS_MEDIA_HANDLER_SRC(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrc))
#define KMS_IS_MEDIA_HANDLER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_SRC))
#define KMS_MEDIA_HANDLER_SRC_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcInterface))

typedef struct _KmsMediaHandlerSrc		KmsMediaHandlerSrc; /* dummy object */
typedef struct _KmsMediaHandlerSrcInterface	KmsMediaHandlerSrcInterface;

struct _KmsMediaHandlerSrcInterface {
	GTypeInterface parent_iface;

	/* Virtual methods */
	/* TODO: Add methods */
};

GType kms_media_handler_src_get_type (void);

#endif /* __KMS_MEDIA_HANDLER_SRC_H__ */
