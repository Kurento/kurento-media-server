#include <kms-core.h>
#include <rtmp/kms-rtmp-sender.h>
#include <rtmp/kms-rtmp-session.h>
#include "internal/kms-utils.h"

#define KMS_RTMP_SENDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTMP_SENDER, KmsRtmpSenderPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTMP_SENDER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTMP_SENDER(obj)->priv->mutex))

struct _KmsRtmpSenderPriv {
	GMutex *mutex;

	KmsRtmpSession *remote_spec;
};

enum {
	PROP_0,

	PROP_REMOTE_SPEC,
};

G_DEFINE_TYPE(KmsRtmpSender, kms_rtmp_sender, KMS_TYPE_MEDIA_HANDLER_SINK)

static void
dispose_remote_spec(KmsRtmpSender *self) {
	if (self->priv->remote_spec != NULL) {
		g_object_unref(self->priv->remote_spec);
		self->priv->remote_spec = NULL;
	}
}

static void
constructed(GObject *object) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->constructed(object);

	g_return_if_fail(self->priv->remote_spec != NULL);

	g_print("TODO: create media chain");
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	switch (property_id) {
		case PROP_REMOTE_SPEC:
			LOCK(self);
			dispose_remote_spec(self);
			self->priv->remote_spec = g_value_dup_object(value);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	switch (property_id) {
		case PROP_REMOTE_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->remote_spec);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
dispose(GObject *object) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	LOCK(self);
	dispose_remote_spec(self);
	UNLOCK(self);

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtmpSender *self = KMS_RTMP_SENDER(object);

	g_mutex_free(self->priv->mutex);

	G_OBJECT_CLASS(kms_rtmp_sender_parent_class)->finalize(object);
}

static void
kms_rtmp_sender_class_init(KmsRtmpSenderClass *klass) {
	GParamSpec *pspec;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsRtmpSenderPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->constructed = constructed;

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_rtmp_sender_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_rtmp_sender_parent_class)->numpadtemplates;

	pspec = g_param_spec_object("remote-spec", "Remote Session Spec",
					"Remote Session Description",
					KMS_TYPE_SDP_SESSION,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_REMOTE_SPEC, pspec);
}

static void
kms_rtmp_sender_init(KmsRtmpSender *self) {
	self->priv = KMS_RTMP_SENDER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->remote_spec = NULL;
}
