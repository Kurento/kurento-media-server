#include <kms-core.h>

#define KMS_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_CONNECTION, KmsConnectionPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_CONNECTION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_CONNECTION(obj)->priv->mutex)))

struct _KmsConnectionPriv {
	GStaticMutex mutex;
	gchar *id;
	gboolean finished;
	KmsEndpoint *endpoint;
	KmsConnectionMode audio_mode;
	KmsConnectionMode video_mode;
};

enum {
	PROP_0,

	PROP_ID,
	PROP_ENDPOINT
};

G_DEFINE_TYPE(KmsConnection, kms_connection, G_TYPE_OBJECT)

static void dispose_endpoint(KmsConnection *self);

static void
endpoint_unref(gpointer data, GObject *session) {
	KmsConnection *self = KMS_CONNECTION(data);

	g_warn_if_reached();
	LOCK(self);
	if (self->priv->endpoint == KMS_ENDPOINT(session)) {
		self->priv->endpoint = NULL;
	}
	UNLOCK(self);
}

static void
free_id(KmsConnection *self) {
	if (self->priv->id != NULL) {
		g_free(self->priv->id);
		self->priv->id = NULL;
	}
}

static void
dispose_endpoint(KmsConnection *self) {
	if (self->priv->endpoint != NULL) {
		g_object_weak_unref(G_OBJECT(self->priv->endpoint),
							endpoint_unref, self);
		self->priv->endpoint = NULL;
	}
}

static gboolean
do_set_mode(KmsConnection *self, KmsConnectionMode mode,
					KmsMediaType type, GError **err) {
	gboolean ret;

	if (KMS_CONNECTION_GET_CLASS(self)->mode_changed == NULL) {
		g_warn_if_reached();
		SET_ERROR(err, KMS_CONNECTION_ERROR,
				KMS_CONNECTION_ERROR_NOT_IMPLEMENTED,
				"Class %s does not reimplement "
				"mode_changed method",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));

		return FALSE;
	}

	ret = KMS_CONNECTION_GET_CLASS(self)->mode_changed(self, mode, type,
									err);

	if (ret) {
		switch(type) {
		case KMS_MEDIA_TYPE_AUDIO:
			self->priv->audio_mode = mode;
			break;
		case KMS_MEDIA_TYPE_VIDEO:
			self->priv->video_mode = mode;
			break;
		default:
			break;
		}
	}

	return ret;
}

gboolean
kms_connection_set_mode(KmsConnection *self, KmsConnectionMode mode,
					KmsMediaType type, GError **err) {
	gboolean ret;
	LOCK(self);

	if (self->priv->finished) {
		SET_ERROR(err, KMS_CONNECTION_ERROR,
					KMS_CONNECTION_ERROR_TERMINATED,
					"Connection %s has been "
					"terminated and cannot be used.",
					self->priv->id);
		ret = FALSE;
		goto end;
	}

	ret = do_set_mode(self, mode, type, err);

end:
	UNLOCK(self);
	return ret;
}

KmsConnectionMode
kms_connection_get_mode(KmsConnection *self, KmsMediaType type) {
	switch (type) {
	case KMS_MEDIA_TYPE_AUDIO:
		return self->priv->audio_mode;
	case KMS_MEDIA_TYPE_VIDEO:
		return self->priv->video_mode;
	default:
		return KMS_CONNECTION_MODE_INACTIVE;
	}
}

gboolean
kms_connection_terminate(KmsConnection *self, GError **err) {
	gboolean ret;

	LOCK(self);
	self->priv->finished = TRUE;
	ret = do_set_mode(self, KMS_CONNECTION_MODE_INACTIVE,
						KMS_MEDIA_TYPE_AUDIO, err);
	if (ret)
		ret = do_set_mode(self, KMS_CONNECTION_MODE_INACTIVE,
						KMS_MEDIA_TYPE_VIDEO, err);
	UNLOCK(self);
	G_OBJECT_GET_CLASS(self)->dispose(G_OBJECT(self));

	return ret;
}

gboolean
kms_connection_connect_to_remote(KmsConnection *self, KmsSessionSpec *session,
					gboolean local_offerer, GError **err) {

	if (!KMS_IS_CONNECTION(self))
		return FALSE;

	if (KMS_CONNECTION_GET_CLASS(self)->connect_to_remote == NULL) {
		g_warn_if_reached();
		SET_ERROR(err, KMS_CONNECTION_ERROR,
				KMS_CONNECTION_ERROR_NOT_IMPLEMENTED,
				"Class %s does not reimplement "
				"connect_to_remote method",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));

		return FALSE;
	}

	return KMS_CONNECTION_GET_CLASS(self)->connect_to_remote(self, session,
							local_offerer, err);
}

gboolean
kms_connection_connect(KmsConnection *self, KmsConnection *other,
					KmsMediaType type, GError **err) {
	if (KMS_CONNECTION_GET_CLASS(self)->connect == NULL) {
		g_warn_if_reached();
		SET_ERROR(err, KMS_CONNECTION_ERROR,
				KMS_CONNECTION_ERROR_NOT_IMPLEMENTED,
				"Class %s does not reimplement "
				"connect_to_remote method",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));

		return FALSE;
	}

	return KMS_CONNECTION_GET_CLASS(self)->connect(self, other, type, err);
}

static void
kms_connection_set_property(GObject  *object, guint property_id,
				const GValue *value, GParamSpec *pspec) {
	KmsConnection *self = KMS_CONNECTION(object);

	switch (property_id) {
		case PROP_0:
			/* Do nothing */
			break;
		case PROP_ID:
			LOCK(self);
			free_id(self);
			self->priv->id = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_ENDPOINT:
			if (!G_VALUE_HOLDS_OBJECT(value))
				break;
			LOCK(self);
			dispose_endpoint(self);
			self->priv->endpoint = g_value_get_object(value);
			g_object_weak_ref(G_OBJECT(self->priv->endpoint),
					  endpoint_unref, self);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_connection_get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsConnection *self = KMS_CONNECTION(object);

	switch (property_id) {
		case PROP_ID:
			LOCK(self);
			g_value_set_string(value, self->priv->id);
			UNLOCK(self);
			break;
		case PROP_ENDPOINT:
			LOCK(self);
			g_value_set_object(value, self->priv->endpoint);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_connection_dispose(GObject *object) {
	KmsConnection *self = KMS_CONNECTION(object);

	LOCK(self);
	dispose_endpoint(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_connection_parent_class)->dispose(object);
}

static void
kms_connection_finalize(GObject *object) {
	KmsConnection *self = KMS_CONNECTION(object);

	free_id(self);
	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_connection_parent_class)->finalize(object);
}

static void
kms_connection_class_init(KmsConnectionClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private(klass, sizeof (KmsConnectionPriv));

	gobject_class->set_property = kms_connection_set_property;
	gobject_class->get_property = kms_connection_get_property;
	gobject_class->dispose = kms_connection_dispose;
	gobject_class->finalize = kms_connection_finalize;

	klass->mode_changed = NULL;
	klass->connect_to_remote = NULL;
	klass->connect = NULL;

	pspec = g_param_spec_string("id", "Connection identifier",
					"Gets the connection identifier",
					"", G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_ID, pspec);

	pspec = g_param_spec_object("endpoint", "Connection endpoint",
				"The endpoint the connection belongs to",
				KMS_TYPE_ENDPOINT,
				G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_ENDPOINT, pspec);
}

static void
kms_connection_init(KmsConnection *self) {
	self->priv = KMS_CONNECTION_GET_PRIVATE (self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->id = NULL;
	self->priv->finished = FALSE;
	self->priv->endpoint = NULL;
	self->priv->audio_mode = KMS_CONNECTION_MODE_INACTIVE;
	self->priv->video_mode = KMS_CONNECTION_MODE_INACTIVE;
}
