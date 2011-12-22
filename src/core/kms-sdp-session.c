#include <kms-core.h>

#define KMS_SDP_SESSION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_SDP_SESSION, KmsSdpSessionPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_SDP_SESSION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_SDP_SESSION(obj)->priv->mutex)))

struct _KmsSdpSessionPriv {
	GStaticMutex mutex;
	gchar *addr;
	GValueArray *medias;
	gchar *name;
	glong id;
	glong version;
	gint sdp_version;
	gchar *remote_handler;
	gchar *username;
};

enum {
	PROP_0,

	PROP_ADDR,
	PROP_MEDIAS,
	PROP_NAME,
	PROP_ID,
	PROP_VERSION,
	PROP_SDP_VERSION,
	PROP_REMOTE_HANDLER,
	PROP_USERNAME
};

G_DEFINE_TYPE(KmsSdpSession, kms_sdp_session, G_TYPE_OBJECT)

static void
free_addr(KmsSdpSession *self) {
	if (self->priv->addr != NULL) {
		g_free(self->priv->addr);
		self->priv->addr = NULL;
	}
}

static void
free_name(KmsSdpSession *self) {
	if (self->priv->name != NULL) {
		g_free(self->priv->name);
		self->priv->name = NULL;
	}
}

static void
free_remote_handler(KmsSdpSession *self) {
	if (self->priv->remote_handler != NULL) {
		g_free(self->priv->remote_handler);
		self->priv->remote_handler = NULL;
	}
}

static void
free_username(KmsSdpSession *self) {
	if (self->priv->username != NULL) {
		g_free(self->priv->username);
		self->priv->username = NULL;
	}
}

static void
free_medias(KmsSdpSession *self) {
	if (self->priv->medias != NULL) {
		g_value_array_free(self->priv->medias);
		self->priv->medias = NULL;
	}
}

static void
kms_sdp_session_set_property(GObject  *object, guint property_id,
				const GValue *value, GParamSpec *pspec) {
	KmsSdpSession *self = KMS_SDP_SESSION(object);

	switch (property_id) {
		case PROP_0:
			/* Do nothing */
			break;
		case PROP_ADDR:
			LOCK(self);
			free_addr(self);
			self->priv->addr = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_MEDIAS:{
			GValueArray *va = g_value_get_boxed(value);
			gint i;

			va = g_value_get_boxed(value);
			if (va == NULL)
				break;

			LOCK(self);
			free_medias(self);
			self->priv->medias = g_value_array_copy(va);

			for (i=0; i < va->n_values; i++) {
				GValue *v;
				KmsSdpMedia *media;

				v = g_value_array_get_nth(va, i);
				media = g_value_get_object(v);
				g_object_set(media, "session", self, NULL);
			}

			UNLOCK(self);
			break;
		}
		case PROP_NAME:
			LOCK(self);
			free_name(self);
			self->priv->name = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_SDP_VERSION:
			LOCK(self);
			self->priv->sdp_version = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_REMOTE_HANDLER:
			LOCK(self);
			free_remote_handler(self);
			self->priv->remote_handler = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_USERNAME:
			LOCK(self);
			free_username(self);
			self->priv->username = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_ID:
			LOCK(self);
			free_username(self);
			self->priv->id = g_value_get_long(value);
			UNLOCK(self);
			break;
		case PROP_VERSION:
			LOCK(self);
			free_username(self);
			self->priv->version = g_value_get_long(value);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_sdp_session_get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsSdpSession *self = KMS_SDP_SESSION(object);

	switch (property_id) {
		case PROP_0:
			break;
		case PROP_ADDR:
			LOCK(self);
			g_value_set_string(value, self->priv->addr);
			UNLOCK(self);
			break;
		case PROP_MEDIAS:
			LOCK(self);
			g_value_set_boxed(value, self->priv->medias);
			UNLOCK(self);
			break;
		case PROP_NAME:
			LOCK(self);
			g_value_set_string(value, self->priv->name);
			UNLOCK(self);
			break;
		case PROP_SDP_VERSION:
			LOCK(self);
			g_value_set_int(value, self->priv->sdp_version);
			UNLOCK(self);
			break;
		case PROP_REMOTE_HANDLER:
			LOCK(self);
			g_value_set_string(value, self->priv->remote_handler);
			UNLOCK(self);
			break;
		case PROP_USERNAME:
			LOCK(self);
			g_value_set_string(value, self->priv->username);
			UNLOCK(self);
			break;
		case PROP_ID:
			LOCK(self);
			g_value_set_long(value, self->priv->id);
			UNLOCK(self);
			break;
		case PROP_VERSION:
			LOCK(self);
			g_value_set_long(value, self->priv->version);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

void
kms_sdp_session_dispose(GObject *object) {
	LOCK(object);
	free_medias(KMS_SDP_SESSION(object));
	UNLOCK(object);
	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_sdp_session_parent_class)->dispose(object);
}

void
kms_sdp_session_finalize(GObject *object) {
	KmsSdpSession *self = KMS_SDP_SESSION(object);

	free_addr(self);
	free_name(self);
	free_remote_handler(self);
	free_username(self);
	g_static_mutex_free(&(self->priv->mutex));
	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_sdp_session_parent_class)->finalize(object);
}

KmsSdpSession*
kms_sdp_session_copy(KmsSdpSession *self) {
	KmsSdpMedia *media, *mcopy;
	GValue *orig, vcopy = G_VALUE_INIT;
	KmsSdpSession *copy;
	GValueArray *medias;
	gint i;

	medias = g_value_array_new(0);

	LOCK(self);
	if (self->priv->medias != NULL) {
		for (i = 0; i < self->priv->medias->n_values; i++) {
			orig = g_value_array_get_nth(self->priv->medias, i);
			if (orig == NULL)
				continue;

			g_value_init(&vcopy, KMS_TYPE_SDP_MEDIA);
			media = g_value_get_object(orig);
			mcopy = kms_sdp_media_copy(media);

			g_value_take_object(&vcopy, mcopy);
			g_value_array_append(medias, &vcopy);
			g_value_unset(&vcopy);
		}
	}

	copy = g_object_new(KMS_TYPE_SDP_SESSION,
				"medias", medias,
				"address", self->priv->addr,
				"name", self->priv->name,
				"id", self->priv->id,
				"version", self->priv->version,
				"sdp-version", self->priv->sdp_version,
				"remote-handler", self->priv->remote_handler,
				"username", self->priv->username,
				NULL);
	UNLOCK(self);

	g_value_array_free(medias);

	return copy;
}

static void
kms_sdp_session_class_init(KmsSdpSessionClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec, *media;

	g_type_class_add_private(klass, sizeof (KmsSdpSessionPriv));

	gobject_class->set_property = kms_sdp_session_set_property;
	gobject_class->get_property = kms_sdp_session_get_property;
	gobject_class->dispose = kms_sdp_session_dispose;
	gobject_class->finalize = kms_sdp_session_finalize;

	pspec = g_param_spec_string("address", "Address",
					"Remote address",
					"-",
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_ADDR, pspec);

	media = g_param_spec_object("media", "Media", "A supported media",
					KMS_TYPE_SDP_MEDIA, G_PARAM_READWRITE);

	pspec = g_param_spec_value_array("medias", "Medias",
					"The medias defines in this session",
					media, G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_MEDIAS, pspec);

	pspec = g_param_spec_string("name", "Session name",
					"The session name",
					"",
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_NAME, pspec);

	pspec = g_param_spec_int("sdp_version", "SDP Version",
					"SDP protocol version",
					0, 2, 0,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_SDP_VERSION, pspec);

	pspec = g_param_spec_string("remote_handler", "Remote handler",
					"Remote handler",
					"",
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_REMOTE_HANDLER, pspec);

	pspec = g_param_spec_string("username", "Username",
					"User name",
					"-",
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_USERNAME, pspec);

	pspec = g_param_spec_long("id", "Session identifier",
					"Session identifier",
					0L, G_MAXLONG, 0L,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_ID, pspec);

	pspec = g_param_spec_long("version", "Session Version",
					"Session Version",
					0L, G_MAXLONG, 0L,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_VERSION, pspec);
}

static void
kms_sdp_session_init(KmsSdpSession *self) {
	self->priv = KMS_SDP_SESSION_GET_PRIVATE (self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->addr = NULL;
	self->priv->medias = NULL;
	self->priv->name = NULL;
	self->priv->id = 0L;
	self->priv->version = 0L;
	self->priv->sdp_version = 0;
	self->priv->remote_handler = NULL;
	self->priv->username = NULL;
}
