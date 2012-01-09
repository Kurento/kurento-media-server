#if !defined (__KMS_RTP_H_INSIDE__)
#error "Only <rtp/kms-rtp.h> can be included directly."
#endif

#ifndef __KMS_RTP_SENDER_H__
#define __KMS_RTP_SENDER_H__

#include <glib-object.h>

#define KMS_TYPE_RTP_SENDER		(kms_rtp_sender_get_type())
#define KMS_RTP_SENDER(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_RTP_SENDER, KmsRtpSender))
#define KMS_IS_RTP_SENDER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_RTP_SENDER))
#define KMS_RTP_SENDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsRtpSenderClass))
#define KMS_IS_RTP_SENDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTP_SENDER))
#define KMS_RTP_SENDER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTP_SENDER, KmsRtpSenderClass))

typedef struct _KmsRtpSender		KmsRtpSender;
typedef struct _KmsRtpSenderClass	KmsRtpSenderClass;
typedef struct _KmsRtpSenderPriv	KmsRtpSenderPriv;

struct _KmsRtpSender {
	GObject parent_instance;

	/* instance members */

	KmsRtpSenderPriv *priv;
};

struct _KmsRtpSenderClass {
	GObjectClass parent_class;

	/* class members */
};

GType kms_rtp_sender_get_type (void);

void kms_rtp_sender_terminate(KmsRtpSender *self);

#endif /* __KMS_RTP_SENDER_H__ */
