#include <kms-core.h>
#include <rtp/kms-rtp.h>

#define KMS_RTP_RECEIVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_RECEIVER, KmsRtpReceiverPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTP_RECEIVER(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTP_RECEIVER(obj)->priv->mutex)))

struct _KmsRtpReceiverPriv {
	GStaticMutex mutex;
	KmsSdpMedia *local_spec;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsRtpReceiver, kms_rtp_receiver, G_TYPE_OBJECT)

static void
dispose_local_spec(KmsRtpReceiver *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
}

static void
dispose(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	LOCK(self);
	dispose_local_spec(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_receiver_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_receiver_parent_class)->finalize(object);
}

static void
kms_rtp_receiver_class_init(KmsRtpReceiverClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsRtpReceiverPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
}

static void
kms_rtp_receiver_init(KmsRtpReceiver *self) {
	self->priv = KMS_RTP_RECEIVER_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->local_spec = NULL;
}
