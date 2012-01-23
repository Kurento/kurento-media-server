#include <rtmp/kms-rtmp-session.h>

#include <glib.h>
#include <stdlib.h>

#define KMS_RTMP_SESSION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTMP_SESSION, KmsRtmpSessionPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTMP_SESSION(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTMP_SESSION(obj)->priv->mutex))

struct _KmsRtmpSessionPriv {
	GMutex *mutex;
	gchar *url;
	gchar *offerer;
	gchar *answerer;
	gint width;
	gint height;
	gint fps_num;
	gint fps_denom;
};

enum {
	PROP_0,

	PROP_URL,
	PROP_OFFERER,
	PROP_ANSWERER,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_FPS_NUM,
	PROP_FPS_DENOM,
};

G_DEFINE_TYPE(KmsRtmpSession, kms_rtmp_session, G_TYPE_OBJECT)

static void
free_url(KmsRtmpSession *self) {
	if (self->priv->url != NULL) {
		g_free(self->priv->url);
		self->priv->url = NULL;
	}
}

static void
free_offerer(KmsRtmpSession *self) {
	if (self->priv->offerer != NULL) {
		g_free(self->priv->offerer);
		self->priv->offerer = NULL;
	}
}

static void
free_answerer(KmsRtmpSession *self) {
	if (self->priv->answerer != NULL) {
		g_free(self->priv->answerer);
		self->priv->answerer = NULL;
	}
}

static void
kms_rtmp_session_set_property(GObject  *object, guint property_id,
				const GValue *value, GParamSpec *pspec) {
	KmsRtmpSession *self = KMS_RTMP_SESSION(object);

	switch (property_id) {
		case PROP_0:
			/* Do nothing */
			break;
		case PROP_URL:
			LOCK(self);
			free_url(self);
			self->priv->url = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_OFFERER:
			LOCK(self);
			free_offerer(self);
			self->priv->offerer = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_ANSWERER:
			LOCK(self);
			free_answerer(self);
			self->priv->answerer = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_WIDTH:
			LOCK(self);
			self->priv->width = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_HEIGHT:
			LOCK(self);
			self->priv->height = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_FPS_NUM:
			LOCK(self);
			self->priv->fps_num = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_FPS_DENOM:
			LOCK(self);
			self->priv->fps_denom = g_value_get_int(value);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_rtmp_session_get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsRtmpSession *self = KMS_RTMP_SESSION(object);

	switch (property_id) {
		case PROP_URL:
			LOCK(self);
			g_value_set_string(value, self->priv->url);
			UNLOCK(self);
			break;
		case PROP_OFFERER:
			LOCK(self);
			g_value_set_string(value, self->priv->offerer);
			UNLOCK(self);
			break;
		case PROP_ANSWERER:
			LOCK(self);
			g_value_set_string(value, self->priv->answerer);
			UNLOCK(self);
			break;
		case PROP_WIDTH:
			LOCK(self);
			g_value_set_int(value, self->priv->width);
			UNLOCK(self);
			break;
		case PROP_HEIGHT:
			LOCK(self);
			g_value_set_int(value, self->priv->height);
			UNLOCK(self);
			break;
		case PROP_FPS_NUM:
			LOCK(self);
			g_value_set_int(value, self->priv->fps_num);
			UNLOCK(self);
			break;
		case PROP_FPS_DENOM:
			LOCK(self);
			g_value_set_int(value, self->priv->fps_denom);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

KmsRtmpSession*
kms_rtmp_session_copy(KmsRtmpSession *self) {
	KmsRtmpSession *copy;

	copy = g_object_new(KMS_TYPE_RTMP_SESSION,
				"url", self->priv->url,
				"offerer", self->priv->offerer,
				"answerer", self->priv->answerer,
				"width", self->priv->width,
				"height", self->priv->height,
				"fps_num", self->priv->fps_num,
				"fps_denom", self->priv->fps_denom,
				NULL);

	return copy;
}

gchar *
kms_rtmp_session_to_string(KmsRtmpSession *self) {
	GString *str;
	gchar *ret;

	LOCK(self);
	str = g_string_sized_new(20);

	g_string_append(str, "url=");
	if (self->priv->url != NULL)
		g_string_append_printf(str, "%s", self->priv->url);

	g_string_append(str, ";offerer=");
	if (self->priv->offerer != NULL)
		g_string_append_printf(str, "%s", self->priv->offerer);

	g_string_append(str, ";answerer=");
	if (self->priv->answerer != NULL)
		g_string_append_printf(str, "%s", self->priv->answerer);

	g_string_append(str, ";w=");
	if (self->priv->width != -1)
		g_string_append_printf(str, "%d", self->priv->width);

	g_string_append(str, ";h=");
	if (self->priv->height != -1)
		g_string_append_printf(str, "%d", self->priv->height);

	g_string_append(str, ";fps=");
	if (self->priv->fps_denom != -1 && self->priv->fps_num != -1)
		g_string_append_printf(str, "%d/%d", self->priv->fps_denom,
							self->priv->fps_num);
	UNLOCK(self);

	ret = str->str;

	g_string_free(str, FALSE);

	return ret;
}

void
kms_rtmp_session_dispose(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_rtmp_session_parent_class)->dispose(object);
}

void
kms_rtmp_session_finalize(GObject *object) {
	KmsRtmpSession *self = KMS_RTMP_SESSION(object);

	free_url(self);
	free_offerer(self);
	free_answerer(self);
	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_rtmp_session_parent_class)->finalize(object);
}

KmsSdpSession*
kms_rtmp_session_get_sdp_session(KmsRtmpSession *session) {
	KmsSdpSession *sdp_session;
	GValueArray *medias;
	KmsSdpMedia *media;
	GValueArray *payloads;
	KmsSdpPayload *payload;
	gchar *fmtp;
	GValue pvalue = G_VALUE_INIT;
	GValue mvalue = G_VALUE_INIT;

	fmtp = kms_rtmp_session_to_string(session);

	payload = g_object_new(KMS_TYPE_SDP_PAYLOAD, "name", "RTMP",
							"fmtp", fmtp,
							"payload", 96, NULL);
	g_free(fmtp);
	payloads = g_value_array_new(1);
	g_value_init(&pvalue, KMS_TYPE_SDP_PAYLOAD);
	g_value_take_object(&pvalue, payload);
	g_value_array_append(payloads, &pvalue);
	g_value_unset(&pvalue);

	media = g_object_new(KMS_TYPE_SDP_MEDIA, "payloads", payloads, NULL);
	g_value_array_free(payloads);
	medias = g_value_array_new(1);
	g_value_init(&mvalue, KMS_TYPE_SDP_MEDIA);
	g_value_take_object(&mvalue, media);
	g_value_array_append(medias, &mvalue);
	g_value_unset(&mvalue);

	sdp_session = g_object_new(KMS_TYPE_SDP_SESSION, "medias", medias, NULL);
	g_value_array_free(medias);

	return sdp_session;
}

static void
kms_rtmp_session_class_init(KmsRtmpSessionClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private(klass, sizeof (KmsRtmpSessionPriv));

	gobject_class->set_property = kms_rtmp_session_set_property;
	gobject_class->get_property = kms_rtmp_session_get_property;
	gobject_class->dispose = kms_rtmp_session_dispose;
	gobject_class->finalize = kms_rtmp_session_finalize;

	pspec = g_param_spec_string("url", "Url",
					"Rtmp url to publish and play media",
					"", G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_URL, pspec);

	pspec = g_param_spec_string("offerer", "Offerer",
					"Stream identifier of offerer peer",
					"", G_PARAM_CONSTRUCT |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_OFFERER, pspec);

	pspec = g_param_spec_string("answerer", "Answerer",
					"Stream identifier of answerer peer",
					"", G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_ANSWERER, pspec);

	pspec = g_param_spec_int("width", "Width",
						"Video width",
						-1, G_MAXINT, -1,
						G_PARAM_CONSTRUCT_ONLY |
						G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_WIDTH, pspec);

	pspec = g_param_spec_int("height", "Height",
						"Video height",
						-1, G_MAXINT, -1,
						G_PARAM_CONSTRUCT_ONLY |
						G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_HEIGHT, pspec);

	pspec = g_param_spec_int("fps_num", "Fps_num",
						"Fps numerator",
						-1, G_MAXINT, -1,
						G_PARAM_CONSTRUCT_ONLY |
						G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_FPS_NUM, pspec);

	pspec = g_param_spec_int("fps_denom", "Fps_denom",
						"Fps denominator",
						-1, G_MAXINT, -1,
						G_PARAM_CONSTRUCT_ONLY |
						G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_FPS_DENOM, pspec);
}

KmsRtmpSession*
kms_rtmp_session_create_from_string(const gchar *str) {
	KmsRtmpSession *session;
	gchar **tokens;
	gchar *iter;
	int i;

	gchar *url = NULL, *offerer = NULL, *answerer = NULL;
	gint width = -1, height = -1, fps_num = -1, fps_denom = -1;

	tokens = g_strsplit(str, ";", 0);

	for (i = 0, iter = tokens[0]; iter != NULL; i++,iter=tokens[i]) {
		gchar **id_value;
		gchar *id, *value;

		id_value = g_strsplit(iter, "=", 2);

		id = id_value[0];
		if (id != NULL)
			goto end;

		value = id_value[1];
		if (value != NULL)
			goto end;

		if (g_strcmp0(id, "url") == 0) {
			url = g_strdup(value);
		} else if (g_strcmp0(id, "offerer") == 0) {
			offerer = g_strdup(value);
		} else if (g_strcmp0(id, "anwserer") == 0) {
			answerer = g_strdup(value);
		} else if (g_strcmp0(id, "w") == 0) {
			width = atoi(value);
		} else if (g_strcmp0(id, "h") == 0) {
			height = atoi(value);
		} else if (g_strcmp0(id, "fps") == 0) {
			gchar **fraction;
			gchar *num;
			gchar *denom;

			fraction = g_strsplit(value, "/", 2);

			num = fraction[0];
			if (num != NULL) {
				denom = fraction[1];
				if (denom != NULL) {
					fps_num = atoi(num);
					fps_denom = atoi(denom);
				}
			}

			g_strfreev(fraction);
		}

end:
		g_strfreev(id_value);
	}

	if (url == NULL)
		url = g_strdup("");

	if (offerer == NULL)
		offerer = g_strdup("");

	if (answerer == NULL)
		answerer = g_strdup("");

	session = g_object_new(KMS_TYPE_RTMP_SESSION,
						"url", url,
						"offerer", offerer,
						"answerer", answerer,
						"width", width,
						"height", height,
						"fps_num", fps_num,
						"fps_denom", fps_denom,
						NULL);

	g_free(url);
	g_free(offerer);
	g_free(answerer);
	g_strfreev(tokens);

	return session;
}

static void
kms_rtmp_session_init(KmsRtmpSession *self) {
	self->priv = KMS_RTMP_SESSION_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->url = NULL;
	self->priv->offerer = NULL;
	self->priv->answerer = NULL;
	self->priv->width = -1;
	self->priv->height = -1;
	self->priv->fps_denom = -1;
	self->priv->fps_num = -1;
}
