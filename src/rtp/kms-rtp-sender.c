#include <kms-core.h>
#include <rtp/kms-rtp.h>

#define KMS_RTP_SENDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_SENDER, KmsRtpSenderPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTP_SENDER(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTP_SENDER(obj)->priv->mutex)))

struct _KmsRtpSenderPriv {
	GStaticMutex mutex;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsRtpSender, kms_rtp_sender, G_TYPE_OBJECT)

static void
kms_rtp_sender_class_init(KmsRtpSenderClass *klass) {
	g_type_class_add_private(klass, sizeof(KmsRtpSenderPriv));
}

static void
kms_rtp_sender_init(KmsRtpSender *self) {
	self->priv = KMS_RTP_SENDER_GET_PRIVATE(self);
}
