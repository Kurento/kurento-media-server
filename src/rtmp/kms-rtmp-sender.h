#ifndef __KMS_RTMP_SENDER_H__
#define __KMS_RTMP_SENDER_H__

#include <kms-core.h>
#include <glib-object.h>

#define KMS_TYPE_RTMP_SENDER		(kms_rtmp_sender_get_type())
#define KMS_RTMP_SENDER(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_RTMP_SENDER, KmsRtmpSender))
#define KMS_IS_RTMP_SENDER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_RTMP_SENDER))
#define KMS_RTMP_SENDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsRtmpSenderClass))
#define KMS_IS_RTMP_SENDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTMP_SENDER))
#define KMS_RTMP_SENDER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTMP_SENDER, KmsRtmpSenderClass))

typedef struct _KmsRtmpSender		KmsRtmpSender;
typedef struct _KmsRtmpSenderClass	KmsRtmpSenderClass;
typedef struct _KmsRtmpSenderPriv	KmsRtmpSenderPriv;

struct _KmsRtmpSender {
	KmsMediaHandlerSink parent_instance;

	/* instance members */

	KmsRtmpSenderPriv *priv;
};

struct _KmsRtmpSenderClass {
	KmsMediaHandlerSinkClass parent_class;

	/* class members */
};

GType kms_rtmp_sender_get_type (void);

#endif /* __KMS_RTMP_SENDER_H__ */
