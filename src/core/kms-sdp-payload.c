#include <kms-core.h>

#define KMS_SDP_PAYLOAD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_SDP_PAYLOAD, KmsSdpPayloadPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_SDP_PAYLOAD(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_SDP_PAYLOAD(obj)->priv->mutex)))

struct _KmsSdpPayloadPriv {
	GStaticMutex mutex;
	gchar *name;
	gint clockrate;
	gint payload;
	KmsSdpMedia *media;

};

enum {
	PROP_0,

	PROP_PAYLOAD,
	PROP_NAME,
	PROP_CLOCKRATE,
	PROP_MEDIA
};

G_DEFINE_TYPE(KmsSdpPayload, kms_sdp_payload, G_TYPE_OBJECT)

static void dispose_media(KmsSdpPayload *self);

void
media_unref(gpointer data, GObject *media) {
	KmsSdpPayload *self = KMS_SDP_PAYLOAD(data);

	LOCK(self);
	if (self->priv->media == KMS_SDP_MEDIA(media)) {
		dispose_media(self);
	}
	UNLOCK(self);
}

static void
free_name(KmsSdpPayload *self) {
	if (self->priv->name != NULL) {
		g_free(self->priv->name);
		self->priv->name = NULL;
	}
}

static void
dispose_media(KmsSdpPayload *self) {
	if (self->priv->media != NULL) {
		g_object_weak_unref(G_OBJECT(self->priv->media), media_unref,
									self);
		self->priv->media = NULL;
	}
}

static void
kms_sdp_payload_set_property(GObject  *object, guint property_id,
				const GValue *value, GParamSpec *pspec) {
	KmsSdpPayload *self = KMS_SDP_PAYLOAD(object);

	switch (property_id) {
		case PROP_0:
			/* Do nothing */
			break;
		case PROP_NAME:
			LOCK(self);
			free_name(self);
			self->priv->name = g_value_dup_string(value);
			UNLOCK(self);
			break;
		case PROP_CLOCKRATE:
			LOCK(self);
			self->priv->clockrate = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_PAYLOAD:
			LOCK(self);
			self->priv->payload = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_MEDIA:
			if (!G_VALUE_HOLDS_OBJECT(value))
				break;
			LOCK(self);
			dispose_media(self);
			self->priv->media = g_value_get_object(value);
			g_object_weak_ref(G_OBJECT(self->priv->media),
							media_unref, self);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_sdp_payload_get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsSdpPayload *self = KMS_SDP_PAYLOAD(object);

	switch (property_id) {
		case PROP_NAME:
			LOCK(self);
			g_value_set_string(value, self->priv->name);
			UNLOCK(self);
			break;
		case PROP_CLOCKRATE:
			LOCK(self);
			g_value_set_int(value, self->priv->clockrate);
			UNLOCK(self);
			break;
		case PROP_PAYLOAD:
			LOCK(self);
			g_value_set_int(value, self->priv->payload);
			UNLOCK(self);
			break;
		case PROP_MEDIA:
			LOCK(self);
			g_value_set_object(value, self->priv->media);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

gboolean
kms_sdp_payload_equals(KmsSdpPayload *a, KmsSdpPayload *b) {

	if (a->priv->payload < 96 && a->priv->payload == b->priv->payload)
		return TRUE;

	return (g_ascii_strcasecmp(a->priv->name, b->priv->name) == 0) &&
				a->priv->clockrate == b->priv->clockrate;
}

KmsSdpPayload*
kms_sdp_payload_copy(KmsSdpPayload *self) {
	KmsSdpPayload *copy;

	copy = g_object_new(KMS_TYPE_SDP_PAYLOAD,
				"payload", self->priv->payload,
				"name", self->priv->name,
				"clockrate", self->priv->clockrate,
				NULL);

	return copy;
}

gchar *
kms_sdp_payload_to_string(KmsSdpPayload *self) {
	GString *str;
	gchar *ret;

	if (self->priv->name == NULL || g_strcmp0(self->priv->name, "") == 0) {
		return g_strdup("");
	}

	str = g_string_sized_new(20);

	g_string_append_printf(str, "a=rtpmap:%d %s/%d", self->priv->payload,
							self->priv->name,
							self->priv->clockrate);

	/* TODO: Add format encodign parameters */

	g_string_append(str, "\r\n");

	/* TODO: Add format parameters */

	ret = str->str;

	g_string_free(str, FALSE);

	return ret;
}

void
kms_sdp_payload_dispose(GObject *object) {
	dispose_media(KMS_SDP_PAYLOAD(object));

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_sdp_payload_parent_class)->dispose(object);
}

void
kms_sdp_payload_finalize(GObject *object) {
	KmsSdpPayload *self = KMS_SDP_PAYLOAD(object);

	free_name(self);
	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_sdp_payload_parent_class)->finalize(object);
}

static void
kms_sdp_payload_class_init(KmsSdpPayloadClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private(klass, sizeof (KmsSdpPayloadPriv));

	gobject_class->set_property = kms_sdp_payload_set_property;
	gobject_class->get_property = kms_sdp_payload_get_property;
	gobject_class->dispose = kms_sdp_payload_dispose;
	gobject_class->finalize = kms_sdp_payload_finalize;

	pspec = g_param_spec_string("name", "Encoding name",
					"The name of the coded to use",
					"", G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_NAME, pspec);

	pspec = g_param_spec_int("payload", "Codec identifier",
				"The identification of the codec",
				0, 127, 0,
				G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_PAYLOAD, pspec);

	pspec = g_param_spec_int("clockrate", "Clockrate",
				 "The clock rate of the codec",
			  0, G_MAXINT, 0,
			  G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_CLOCKRATE, pspec);

	pspec = g_param_spec_object("media", "Media",
				"Media description that this payload belongs to",
				KMS_TYPE_SDP_MEDIA,
				G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_MEDIA, pspec);
}

static void
kms_sdp_payload_init(KmsSdpPayload *self) {
	self->priv = KMS_SDP_PAYLOAD_GET_PRIVATE (self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->name = NULL;
	self->priv->clockrate = 0;
	self->priv->payload = 0;
	self->priv->media = NULL;
}
