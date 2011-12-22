#include <kms-core.h>
#include <rtp/kms-rtp.h>

#define KMS_RTP_RECEIVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_RECEIVER, KmsRtpReceiverPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTP_RECEIVER(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTP_RECEIVER(obj)->priv->mutex)))

G_LOCK_DEFINE_STATIC(class_lock);

struct _KmsRtpReceiverPriv {
	GStaticMutex mutex;
	KmsSdpMedia *local_spec;
	GstElement *pipe;
	GstElement *bin;
};

enum {
	PROP_0,

	PROP_LOCAL_SPEC,
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
dispose_pipeline(KmsRtpReceiver *self) {
	if (self->priv->pipe != NULL) {
		g_object_unref(self->priv->pipe);
		self->priv->pipe = NULL;
	}
}

static void
dispose_bin(KmsRtpReceiver *self) {
	if (self->priv->bin != NULL) {
		gst_bin_remove(GST_BIN(self->priv->pipe), self->priv->bin);
		gst_element_set_state(self->priv->bin, GST_STATE_NULL);
		g_object_unref(self->priv->bin);
		self->priv->bin = NULL;
	}
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			dispose_local_spec(self);
			self->priv->local_spec = g_value_dup_object(value);
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
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->local_spec);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static gchar*
get_name() {
	static glong count = 0;
	gchar *name;

	G_LOCK(class_lock);
	name = g_strdup_printf("%s-%ld", g_type_name(KMS_TYPE_RTP_RECEIVER),
								count++);
	G_UNLOCK(class_lock);

	return name;
}

static void
constructed(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);
	GstElement *pipe, *bin;
	gchar *name;

	name = get_name();
	bin = gst_bin_new(name);
	self->priv->bin = g_object_ref(bin);
	g_free(name);

	g_object_set(bin, "async-handling", TRUE, NULL);
	GST_OBJECT_FLAG_SET(bin, GST_ELEMENT_LOCKED_STATE);
	gst_element_set_state(bin, GST_STATE_PLAYING);

	pipe = kms_get_pipeline();
	gst_bin_add(GST_BIN(pipe), bin);
	self->priv->pipe = pipe;
}

static void
dispose(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	LOCK(self);
	dispose_local_spec(self);
	dispose_bin(self);
	dispose_pipeline(self);
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
	GParamSpec *pspec;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsRtpReceiverPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->constructed = constructed;

	pspec = g_param_spec_object("local-spec", "Local Session Spec",
					"Local Session Spec",
					KMS_TYPE_SDP_SESSION,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(object_class, PROP_LOCAL_SPEC, pspec);
}

static void
kms_rtp_receiver_init(KmsRtpReceiver *self) {
	self->priv = KMS_RTP_RECEIVER_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->local_spec = NULL;
	self->priv->pipe = NULL;
	self->priv->bin = NULL;
}
