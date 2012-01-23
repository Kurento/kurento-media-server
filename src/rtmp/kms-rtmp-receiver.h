#ifndef __KMS_RTMP_RECEIVER_H__
#define __KMS_RTMP_RECEIVER_H__

#include <glib-object.h>
#include <kms-core.h>

#define KMS_TYPE_RTMP_RECEIVER			(kms_rtmp_receiver_get_type())
#define KMS_RTMP_RECEIVER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_RTMP_RECEIVER, KmsRtmpReceiver))
#define KMS_IS_RTMP_RECEIVER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_RTMP_RECEIVER))
#define KMS_RTMP_RECEIVER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsRtmpReceiverClass))
#define KMS_IS_RTMP_RECEIVER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTMP_RECEIVER))
#define KMS_RTMP_RECEIVER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTMP_RECEIVER, KmsRtmpReceiverClass))

typedef struct _KmsRtmpReceiver		KmsRtmpReceiver;
typedef struct _KmsRtmpReceiverClass	KmsRtmpReceiverClass;
typedef struct _KmsRtmpReceiverPriv	KmsRtmpReceiverPriv;

struct _KmsRtmpReceiver {
	KmsMediaHandlerSrc parent_instance;

	/* instance members */

	KmsRtmpReceiverPriv *priv;
};

struct _KmsRtmpReceiverClass {
	KmsMediaHandlerSrcClass parent_class;

	/* class members */
};

GType kms_rtmp_receiver_get_type (void);

#endif /* __KMS_RTMP_RECEIVER_H__ */
