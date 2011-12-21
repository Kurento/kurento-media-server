#include <kms-core.h>
#include <rtp/kms-rtp.h>

#define KMS_RTP_RECEIVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_RECEIVER, KmsRtpReceiverPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTP_RECEIVER(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTP_RECEIVER(obj)->priv->mutex)))

struct _KmsRtpReceiverPriv {
	GStaticMutex mutex;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsRtpReceiver, kms_rtp_receiver, G_TYPE_OBJECT)

static void
kms_rtp_receiver_class_init(KmsRtpReceiverClass *klass) {
	g_type_class_add_private(klass, sizeof(KmsRtpReceiverPriv));
}

static void
kms_rtp_receiver_init(KmsRtpReceiver *self) {
	self->priv = KMS_RTP_RECEIVER_GET_PRIVATE(self);
}
