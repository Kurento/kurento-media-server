#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_MEDIA_HANDLER_SRC_H__
#define __KMS_MEDIA_HANDLER_SRC_H__

#include <glib-object.h>

#define KMS_TYPE_MEDIA_HANDLER_SRC		(kms_media_handler_src_get_type())
#define KMS_MEDIA_HANDLER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrc))
#define KMS_IS_MEDIA_HANDLER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_SRC))
#define KMS_MEDIA_HANDLER_SRC_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsMediaHandlerSrcClass))
#define KMS_IS_MEDIA_HANDLER_SRC_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MEDIA_HANDLER_SRC))
#define KMS_MEDIA_HANDLER_SRC_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcClass))

typedef struct _KmsMediaHandlerSrc		KmsMediaHandlerSrc;
typedef struct _KmsMediaHandlerSrcClass		KmsMediaHandlerSrcClass;
typedef struct _KmsMediaHandlerSrcPriv		KmsMediaHandlerSrcPriv;

struct _KmsMediaHandlerSrc {
	GObject parent_instance;

	/* instance members */

	KmsMediaHandlerSrcPriv *priv;
};

struct _KmsMediaHandlerSrcClass {
	GObjectClass parent_class;

	/* class members */
};

GType kms_media_handler_src_get_type (void);

#endif /* __KMS_MEDIA_HANDLER_SRC_H__ */
