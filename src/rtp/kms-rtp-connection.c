#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"

#define KMS_RTP_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTP_CONNECTION, KmsRtpConnectionPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTP_CONNECTION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTP_CONNECTION(obj)->priv->mutex)))

enum {
	PROP_0,

	PROP_LOCAL_SPEC,
	PROP_DESCRIPTOR,
};

struct _KmsRtpConnectionPriv {
	GStaticMutex mutex;
	KmsSdpSession *local_spec;
	KmsSdpSession *remote_spec;
	KmsSdpSession *neg_local_spec;
	KmsSdpSession *neg_remote_spec;
	KmsSdpSession *descriptor;
	KmsRtpReceiver *receiver;
	KmsRtpSender *sender;
	gboolean initialized;
};

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);
static void media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsRtpConnection, kms_rtp_connection,
				KMS_TYPE_CONNECTION,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init)
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_FACTORY,
					media_handler_factory_iface_init))

static void
dispose_receiver(KmsRtpConnection *self) {
	if (self->priv->receiver != NULL) {
		kms_rtp_receiver_terminate(self->priv->receiver);
		g_object_unref(self->priv->receiver);
		self->priv->receiver = NULL;
	}
}

static void
dispose_sender(KmsRtpConnection *self) {
	if (self->priv->sender != NULL) {
		kms_rtp_sender_terminate(self->priv->sender);
		g_object_unref(self->priv->sender);
		self->priv->sender = NULL;
	}
}

static void
dispose_local_spec(KmsRtpConnection *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
}

static void
dispose_remote_spec(KmsRtpConnection *self) {
	if (self->priv->remote_spec != NULL) {
		g_object_unref(self->priv->remote_spec);
		self->priv->remote_spec = NULL;
	}
}

static void
dispose_neg_local_spec(KmsRtpConnection *self) {
	if (self->priv->neg_local_spec != NULL) {
		g_object_unref(self->priv->neg_local_spec);
		self->priv->neg_local_spec = NULL;
	}
}

static void
dispose_neg_remote_spec(KmsRtpConnection *self) {
	if (self->priv->neg_remote_spec != NULL) {
		g_object_unref(self->priv->neg_remote_spec);
		self->priv->neg_remote_spec = NULL;
	}
}

static void
dispose_descriptor(KmsRtpConnection *self) {
	if (self->priv->descriptor != NULL) {
		g_object_unref(self->priv->descriptor);
		self->priv->descriptor = NULL;
	}
}

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	return KMS_MEDIA_HANDLER_FACTORY(g_object_ref(iface));
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
}

static KmsMediaHandlerSrc*
get_src(KmsMediaHandlerFactory *iface) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(iface);
	return KMS_MEDIA_HANDLER_SRC(self->priv->receiver);
}

static KmsMediaHandlerSink*
get_sink(KmsMediaHandlerFactory *self) {
	g_warning("%s:%d Not implemented", __FILE__, __LINE__);
	return NULL;
}

static void
media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface) {
	iface->get_sink = get_sink;
	iface->get_src = get_src;
}

static void
create_rtp_sender(KmsRtpConnection *self) {
	gint audio_fd, video_fd;

	g_object_get(self->priv->receiver, "audio-fd", &audio_fd,
						"video-fd", &video_fd,
						NULL);

	self->priv->sender = g_object_new(KMS_TYPE_RTP_SENDER,
				"remote-spec", self->priv->neg_remote_spec,
				"audio-fd", audio_fd,
				"video-fd", video_fd,
				NULL);
}

static gboolean
connect_to_remote(KmsConnection *conn, KmsSdpSession *spec, GError **err) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(conn);

	if (!KMS_IS_SDP_SESSION(spec)) {
		SET_ERROR(err, KMS_RTP_CONNECTION_ERROR,
					KMS_RTP_CONNECTION_ERROR_WRONG_VALUE,
					"Given spect has incorrect type");
		return FALSE;
	}

	LOCK(self);
	if (self->priv->remote_spec != NULL) {
		SET_ERROR(err, KMS_RTP_CONNECTION_ERROR,
					KMS_RTP_CONNECTION_ERROR_ALREADY,
					"Remote spec was already set");
		return FALSE;
	}

	self->priv->remote_spec = g_object_ref(spec);

	if (self->priv->initialized) {
		kms_sdp_session_intersect(spec, self->priv->local_spec,
						&(self->priv->neg_remote_spec),
						&(self->priv->neg_local_spec));
	} else {
		kms_sdp_session_intersect(self->priv->local_spec, spec,
						&(self->priv->neg_local_spec),
						&(self->priv->neg_remote_spec));
	}

	dispose_descriptor(self);
	self->priv->descriptor = g_object_ref(self->priv->neg_local_spec);

	/* Create rtpsender */
	create_rtp_sender(self);

	g_object_set(self->priv->receiver, "local-spec", self->priv->descriptor,
									NULL);
	UNLOCK(self);

	return TRUE;
}

static gboolean
mode_changed(KmsConnection *self, KmsConnectionMode mode, KmsMediaType type,
								GError **err) {
	/* TODO: Implement mode changed for RtpConnection */
	return TRUE;
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			dispose_local_spec(self);
			self->priv->local_spec = g_value_dup_object(value);
			if (self->priv->descriptor == NULL &&
					self->priv->local_spec != NULL) {
				self->priv->descriptor = g_object_ref(
							self->priv->local_spec);
			}
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
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->local_spec);
			UNLOCK(self);
			break;
		case PROP_DESCRIPTOR:
			LOCK(self);
			g_value_set_object(value, self->priv->descriptor);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
constructed(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->constructed(object);

	if (kms_get_pipeline() == NULL) {
		g_warning("Kms should be initialized before instantiate "
								"objects");
		g_assert_not_reached();
		return;
	}

	g_return_if_fail(self->priv->local_spec != NULL);

	self->priv->receiver = g_object_new(KMS_TYPE_RTP_RECEIVER,
					"local-spec", self->priv->local_spec,
					NULL);
}

static void
kms_rtp_connection_dispose(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	LOCK(self);
	dispose_local_spec(self);
	dispose_remote_spec(self);
	dispose_neg_local_spec(self);
	dispose_neg_remote_spec(self);
	dispose_descriptor(self);
	dispose_receiver(self);
	dispose_sender(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->dispose(object);
}

static void
kms_rtp_connection_finalize(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->finalize(object);
}

static void
kms_rtp_connection_class_init (KmsRtpConnectionClass *klass) {
	GParamSpec *pspec;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private (klass, sizeof (KmsRtpConnectionPriv));

	KMS_CONNECTION_CLASS(klass)->mode_changed = mode_changed;
	KMS_CONNECTION_CLASS(klass)->connect_to_remote = connect_to_remote;
	gobject_class->dispose = kms_rtp_connection_dispose;
	gobject_class->finalize = kms_rtp_connection_finalize;
	gobject_class->constructed = constructed;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;

	pspec = g_param_spec_object("local-spec", "Local Session Spec",
					"Local Session Spec",
					KMS_TYPE_SDP_SESSION,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_LOCAL_SPEC, pspec);

	pspec = g_param_spec_object("descriptor", "Session descriptor",
					"The current session descriptor",
					KMS_TYPE_SDP_SESSION,
					G_PARAM_READABLE);

	g_object_class_install_property(gobject_class, PROP_DESCRIPTOR, pspec);
}

static void
kms_rtp_connection_init (KmsRtpConnection *self) {
	self->priv = KMS_RTP_CONNECTION_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->local_spec = NULL;
	self->priv->descriptor = NULL;
	self->priv->receiver = NULL;
	self->priv->sender = NULL;
	self->priv->remote_spec = NULL;
	self->priv->initialized = FALSE;
	self->priv->neg_remote_spec = NULL;
	self->priv->neg_local_spec = NULL;
}
