#include <rtp/kms-rtp.h>

#define KMS_RTP_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTP_ENDPOINT, KmsRtpEndpointPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTP_ENDPOINT(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTP_ENDPOINT(obj)->priv->mutex)))

struct _KmsRtpEndpointPriv {
	GStaticMutex mutex;
	KmsSdpSession *local_spec;
};

enum {
	PROP_0,

	PROP_LOCAL_SPEC
};

G_DEFINE_TYPE(KmsRtpEndpoint, kms_rtp_endpoint, KMS_TYPE_ENDPOINT)

static void
dispose_local_spec(KmsRtpEndpoint *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
}

static KmsConnection*
create_connection(KmsEndpoint *object, gchar *name, GError **err) {
	KmsRtpConnection *conn;
	KmsRtpEndpoint *self = KMS_RTP_ENDPOINT(object);

	LOCK(self);
	if (self->priv->local_spec == NULL) {
		UNLOCK(self);
		SET_ERROR(err, KMS_RTP_ENDPOINT_ERROR,
				KMS_ENDPOINT_ERROR_NOT_FOUND,
				"local-spec property is not set in class %s",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
		return NULL;
	}

	conn = g_object_new(KMS_TYPE_RTP_CONNECTION, "id", name,
					"endpoint", self,
					"local-spec", self->priv->local_spec,
					NULL);

	UNLOCK(self);
	g_object_set(G_OBJECT(self), "manager", conn, NULL);
	return KMS_CONNECTION(conn);
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtpEndpoint *self = KMS_RTP_ENDPOINT(object);

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
	KmsRtpEndpoint *self = KMS_RTP_ENDPOINT(object);

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

static void
dispose(GObject *object) {
	KmsRtpEndpoint *self = KMS_RTP_ENDPOINT(object);

	LOCK(self);
	dispose_local_spec(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_endpoint_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtpEndpoint *self = KMS_RTP_ENDPOINT(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_endpoint_parent_class)->finalize(object);
}

static void
kms_rtp_endpoint_class_init(KmsRtpEndpointClass *klass) {
	GParamSpec *pspec;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsRtpEndpointPriv));

	KMS_ENDPOINT_CLASS(klass)->create_connection = create_connection;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;

	pspec = g_param_spec_object("local-spec", "Local Session Spec",
							"Local Session Spec",
							KMS_TYPE_SDP_SESSION,
							G_PARAM_CONSTRUCT |
							G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_LOCAL_SPEC, pspec);
}

static void
kms_rtp_endpoint_init(KmsRtpEndpoint *self) {
	self->priv = KMS_RTP_ENDPOINT_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->local_spec = NULL;
}
