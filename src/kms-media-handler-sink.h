#ifndef __KMS_MEDIA_HANDLER_SINK_H__
#define __KMS_MEDIA_HANDLER_SINK_H__

#include <glib-object.h>

#define KMS_TYPE_MEDIA_HANDLER_SINK			(kms_media_handler_sink_get_type())
#define KMS_MEDIA_HANDLER_SINK(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSink))
#define KMS_IS_MEDIA_HANDLER_SINK(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_SINK))
#define KMS_MEDIA_HANDLER_SINK_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSinkInterface))

typedef struct _KmsMediaHandlerSink		KmsMediaHandlerSink; /* dummy object */
typedef struct _KmsMediaHandlerSinkInterface	KmsMediaHandlerSinkInterface;

struct _KmsMediaHandlerSinkInterface {
	GTypeInterface parent_iface;

	/* Virtual methods */
	/* TODO: Add methods */
};

GType kms_media_handler_sink_get_type (void);

#endif /* __KMS_MEDIA_HANDLER_SINK_H__ */
