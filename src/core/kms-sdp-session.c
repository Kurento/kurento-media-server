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

static gchar*
medias_to_string(GValueArray *medias) {
	GString *str = g_string_sized_new(50);
	gchar *ret;
	gint i;

	for (i = 0; i < medias->n_values; i++) {
		gchar *media_str;
		GValue *val;
		KmsSdpMedia *media;

		val = g_value_array_get_nth(medias, i);
		media = g_value_get_object(val);

		media_str = kms_sdp_media_to_string(media);

		g_string_append(str, media_str);
		g_free(media_str);
	}

	ret = str->str;
	g_string_free(str, FALSE);

	return ret;
}

gchar*
kms_sdp_session_to_string(KmsSdpSession *self) {
	gchar *str;
	gchar *medias_desc = "";

	LOCK(self);
	medias_desc = medias_to_string(self->priv->medias);

	str = g_strdup_printf("v=%d\r\n"
				"o=%s %ld %ld IN IPV4 %s\r\n"
				"s=%s\r\n"
				"c=IN IPV4 %s\r\n"
				"t=0 0\r\n"
				"%s",
				self->priv->sdp_version,
				self->priv->username,
				self->priv->id,
				self->priv->version,
				self->priv->addr,
				self->priv->name,
				self->priv->remote_handler,
				medias_desc);
	UNLOCK(self);

	g_free(medias_desc);

	return str;
}

static gboolean
compare_media_types(KmsSdpMedia *answerer, KmsSdpMedia *offerer) {
	KmsMediaType a_type, o_type;

	g_object_get(answerer, "type", &a_type, NULL);
	g_object_get(offerer, "type", &o_type, NULL);

	return (a_type == o_type);
}

void
kms_sdp_session_intersect(KmsSdpSession *answerer, KmsSdpSession *offerer,
						KmsSdpSession **neg_answ,
						KmsSdpSession **neg_off) {
	GValueArray *offerer_medias, *answerer_medias;
	GValueArray *new_answerer_medias, *new_offerer_medias;
	GSList *used = NULL;
	gint i, ii;

	g_return_if_fail(KMS_IS_SDP_SESSION(answerer));
	g_return_if_fail(KMS_IS_SDP_SESSION(offerer));

	answerer_medias = answerer->priv->medias;
	offerer_medias = offerer->priv->medias;

	new_answerer_medias = g_value_array_new(answerer_medias->n_prealloced);
	new_offerer_medias = g_value_array_new(offerer_medias->n_prealloced);

	for (i = 0; i < offerer_medias->n_values; i++) {
		KmsSdpMedia *o_media;
		KmsSdpMedia *new_a_media = NULL, *new_o_media;
		GValue aux = G_VALUE_INIT;

		o_media = g_value_get_object(g_value_array_get_nth(
							offerer_medias, i));

		for (ii = 0; ii < answerer_medias->n_values; ii++) {
			KmsSdpMedia *a_media;

			a_media = g_value_get_object(g_value_array_get_nth(
							answerer_medias, ii));

			if (!compare_media_types(o_media, a_media) ||
					g_slist_find(used, a_media) != NULL)
				continue;

			kms_sdp_media_intersect(a_media, o_media,
						&new_a_media, &new_o_media);

			if (new_a_media != NULL) {
				used = g_slist_prepend(used, a_media);
				break;
			}
		}

		if (new_a_media == NULL) {
			KmsMediaType type;
			g_object_get(o_media, "type", &type, NULL);
			new_a_media = g_object_new(KMS_TYPE_SDP_MEDIA,
							"type", type,
							NULL);
			new_o_media = kms_sdp_media_copy(new_a_media);
		}

		g_value_init(&aux, KMS_TYPE_SDP_MEDIA);
		g_value_take_object(&aux, new_a_media);
		g_value_array_append(new_answerer_medias, &aux);
		g_value_reset(&aux);

		g_value_take_object(&aux, new_o_media);
		g_value_array_append(new_offerer_medias, &aux);
		g_value_unset(&aux);
	}

	*neg_off = g_object_new(KMS_TYPE_SDP_SESSION,
				"medias", new_offerer_medias,
				"address", offerer->priv->addr,
				"name", offerer->priv->name,
				"id", offerer->priv->id,
				"version", offerer->priv->version,
				"sdp-version", offerer->priv->sdp_version,
				"remote-handler", offerer->priv->remote_handler,
				"username", offerer->priv->username,
				NULL);

	*neg_answ = g_object_new(KMS_TYPE_SDP_SESSION,
			       "medias", new_answerer_medias,
				"address", answerer->priv->addr,
				"name", offerer->priv->name,
				"id", offerer->priv->id,
				"version", offerer->priv->version,
				"sdp-version", offerer->priv->sdp_version,
				"remote-handler", answerer->priv->remote_handler,
				"username", offerer->priv->username,
				NULL);

	g_value_array_free(new_answerer_medias);
	g_value_array_free(new_offerer_medias);
	g_slist_free(used);
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

			LOCK(self);
			free_medias(self);
			if (va == NULL)
				self->priv->medias = g_value_array_new(0);
			else
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
