#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_MEDIA_HANDLER_SRC_H__
#define __KMS_MEDIA_HANDLER_SRC_H__

#include <glib-object.h>
#include <gst/gst.h>

#define KMS_TYPE_MEDIA_HANDLER_SRC		(kms_media_handler_src_get_type())
#define KMS_MEDIA_HANDLER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrc))
#define KMS_IS_MEDIA_HANDLER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_SRC))
#define KMS_MEDIA_HANDLER_SRC_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsMediaHandlerSrcClass))
#define KMS_IS_MEDIA_HANDLER_SRC_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MEDIA_HANDLER_SRC))
#define KMS_MEDIA_HANDLER_SRC_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcClass))

G_BEGIN_DECLS

typedef struct _KmsMediaHandlerSrc		KmsMediaHandlerSrc;
typedef struct _KmsMediaHandlerSrcClass		KmsMediaHandlerSrcClass;
typedef struct _KmsMediaHandlerSrcPriv		KmsMediaHandlerSrcPriv;

#define KMS_MEDIA_HANDLER_SRC_ERROR (g_quark_from_string("KmsMediaHandlerSrcError"))

enum {
	KMS_MEDIA_HANDLER_SRC_ERROR_PAD_NOT_FOUND,
	KMS_MEDIA_HANDLER_SRC_ERROR_LINK_ERROR,
};

struct _KmsMediaHandlerSrc {
	GstBin parent_instance;

	/* instance members */

	KmsMediaHandlerSrcPriv *priv;
};

struct _KmsMediaHandlerSrcClass {
	GstBinClass parent_class;

	/* class members */
};

GType kms_media_handler_src_get_type (void);

gboolean kms_media_handler_src_connect(KmsMediaHandlerSrc *self,
				KmsMediaHandlerSink *sink,
				KmsMediaType type, GError **err);

void kms_media_handler_src_set_pad(KmsMediaHandlerSrc *self, GstPad *pad,
					GstElement *tee, KmsMediaType type);

void kms_media_handler_src_set_raw_pad(KmsMediaHandlerSrc *self, GstPad *pad,
				   GstElement *tee, KmsMediaType type);

void kms_media_handler_src_terminate(KmsMediaHandlerSrc *self);

G_END_DECLS

#endif /* __KMS_MEDIA_HANDLER_SRC_H__ */
