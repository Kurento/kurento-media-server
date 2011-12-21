#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_MEDIA_HANDLER_SINK_H__
#define __KMS_MEDIA_HANDLER_SINK_H__

#include <glib-object.h>

#define KMS_TYPE_MEDIA_HANDLER_SINK		(kms_media_handler_sink_get_type())
#define KMS_MEDIA_HANDLER_SINK(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSink))
#define KMS_IS_MEDIA_HANDLER_SINK(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_SINK))
#define KMS_MEDIA_HANDLER_SINK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SINK, KmsMediaHandlerSinkClass))
#define KMS_IS_MEDIA_HANDLER_SINK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MEDIA_HANDLER_SINK))
#define KMS_MEDIA_HANDLER_SINK_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSinkClass))

typedef struct _KmsMediaHandlerSink		KmsMediaHandlerSink;
typedef struct _KmsMediaHandlerSinkClass	KmsMediaHandlerSinkClass;
typedef struct _KmsMediaHandlerSinkPriv		KmsMediaHandlerSinkPriv;

struct _KmsMediaHandlerSink {
	GObject parent_instance;

	/* instance members */

	KmsMediaHandlerSinkPriv *priv;
};

struct _KmsMediaHandlerSinkClass {
	GObjectClass parent_class;

	/* class members */
};

GType kms_media_handler_sink_get_type (void);

#endif /* __KMS_MEDIA_HANDLER_SINK_H__ */
