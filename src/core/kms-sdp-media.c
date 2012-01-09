#include <kms-core.h>

#define KMS_SDP_MEDIA_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_SDP_MEDIA, KmsSdpMediaPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_SDP_MEDIA(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_SDP_MEDIA(obj)->priv->mutex)))

struct _KmsSdpMediaPriv {
	GStaticMutex mutex;
	gint port;
	KmsMediaType type;
	KmsSdpMode mode;
	glong bandwidth;
	GValueArray *payloads;
	KmsSdpSession *session;
};

enum {
	PROP_0,

	PROP_PORT,
	PROP_TYPE,
	PROP_BANDWIDTH,
	PROP_MODE,
	PROP_PAYLOADS,
	PROP_SESSION
};

G_DEFINE_TYPE(KmsSdpMedia, kms_sdp_media, G_TYPE_OBJECT)
static void dispose_session(KmsSdpMedia *self);

static void
session_unref(gpointer data, GObject *session) {
	KmsSdpMedia *self = KMS_SDP_MEDIA(data);

	LOCK(self);
	if (self->priv->session == KMS_SDP_SESSION(session)) {
		self->priv->session = NULL;
		g_warn_if_reached();
	}
	UNLOCK(self);
}

static void
free_payloads(KmsSdpMedia *self) {
	if (self->priv->payloads != NULL) {
		g_value_array_free(self->priv->payloads);
		self->priv->payloads = NULL;
	}
}

static void
dispose_session(KmsSdpMedia *self) {
	if (self->priv->session != NULL) {
		g_object_weak_unref(G_OBJECT(self->priv->session),
							session_unref, self);
		self->priv->session = NULL;
	}
}

static GString*
payloads_to_string(GValueArray *payloads, GString *ids) {
	GString *str = g_string_sized_new(50);
	gint i;
	gint id;

	for (i = 0; i < payloads->n_values; i++) {
		gchar *media_str;
		GValue *val;
		KmsSdpPayload *payload;

		val = g_value_array_get_nth(payloads, i);
		payload = g_value_get_object(val);

		media_str = kms_sdp_payload_to_string(payload);

		g_object_get(payload, "payload", &id, NULL);
		g_string_append_printf(ids, " %d", id);

		g_string_append(str, media_str);
		g_free(media_str);
	}

	return str;
}


gchar *
kms_sdp_media_to_string(KmsSdpMedia *self) {
	GString *str = g_string_sized_new(30);
	GString *payids = g_string_sized_new(10);
	GString *paystr;
	gchar *ret;
	GEnumValue *type;
	GEnumClass *eclass;

	LOCK(self);
	eclass = G_ENUM_CLASS(g_type_class_ref(KMS_MEDIA_TYPE));
	type = g_enum_get_value(eclass, self->priv->type);
	g_type_class_unref(eclass);

	paystr = payloads_to_string(self->priv->payloads, payids);

	g_string_append_printf(str, "m=%s %d RTP/AVP%s\r\n%s",
				type->value_nick,
				self->priv->port,
				payids->str,
				paystr->str);

	g_string_free(payids, TRUE);
	g_string_free(paystr, TRUE);

	ret = str->str;
	g_string_free(str, FALSE);
	UNLOCK(self);

	return ret;
}

static void
kms_sdp_media_set_property(GObject  *object, guint property_id,
				const GValue *value, GParamSpec *pspec) {
	KmsSdpMedia *self = KMS_SDP_MEDIA(object);

	switch (property_id) {
		case PROP_0:
			/* Do nothing */
			break;
		case PROP_TYPE:
			LOCK(self);
			self->priv->type = g_value_get_enum(value);
			UNLOCK(self);
			break;
		case PROP_PORT:
			LOCK(self);
			self->priv->port = g_value_get_int(value);
			UNLOCK(self);
			break;
		case PROP_BANDWIDTH:
			LOCK(self);
			self->priv->bandwidth = g_value_get_long(value);
			UNLOCK(self);
			break;
		case PROP_MODE:
			LOCK(self);
			self->priv->mode = g_value_get_enum(value);
			UNLOCK(self);
			break;
		case PROP_PAYLOADS:{
			GValueArray *va;
			gint i;

			va = g_value_get_boxed(value);

			LOCK(self);
			free_payloads(self);
			if (va == NULL)
				self->priv->payloads = g_value_array_new(0);
			else
				self->priv->payloads = g_value_array_copy(va);

			for (i=0; i < self->priv->payloads->n_values; i++) {
				GValue *v;
				KmsSdpPayload *pay;

				v = g_value_array_get_nth(va, i);
				pay = g_value_get_object(v);
				if (pay == NULL)
					continue;
				g_object_set(pay, "media", self, NULL);
			}

			UNLOCK(self);
			break;
		}
		case PROP_SESSION:
			LOCK(self);
			dispose_session(self);
			if (!G_VALUE_HOLDS_OBJECT(value))
				break;
			self->priv->session = g_value_get_object(value);
			g_object_weak_ref(G_OBJECT(self->priv->session),
							session_unref, self);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_sdp_media_get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsSdpMedia *self = KMS_SDP_MEDIA(object);

	switch (property_id) {
		case PROP_TYPE:
			LOCK(self);
			g_value_set_enum(value, self->priv->type);
			UNLOCK(self);
			break;
		case PROP_PORT:
			LOCK(self);
			g_value_set_int(value, self->priv->port);
			UNLOCK(self);
			break;
		case PROP_BANDWIDTH:
			LOCK(self);
			g_value_set_long(value, self->priv->bandwidth);
			UNLOCK(self);
			break;
		case PROP_MODE:
			LOCK(self);
			g_value_set_enum(value, self->priv->mode);
			UNLOCK(self);
			break;
		case PROP_PAYLOADS:
			LOCK(self);
			g_value_set_boxed(value, self->priv->payloads);
			UNLOCK(self);
			break;
		case PROP_SESSION:
			LOCK(self);
			g_value_set_object(value, self->priv->session);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

KmsSdpMedia*
kms_sdp_media_copy(KmsSdpMedia *self) {
	GValueArray *payloads;
	KmsSdpMedia *copy;
	KmsSdpPayload *pay, *pcopy;
	GValue *orig, vcopy = G_VALUE_INIT;
	gint i;

	payloads = g_value_array_new(0);

	LOCK(self);
	if (self->priv->payloads != NULL) {
		for (i = 0; i < self->priv->payloads->n_values; i++) {
			orig = g_value_array_get_nth(self->priv->payloads, i);
			if (orig == NULL)
				continue;

			g_value_init(&vcopy, KMS_TYPE_SDP_PAYLOAD);
			pay = g_value_get_object(orig);
			pcopy = kms_sdp_payload_copy(pay);

			g_value_take_object(&vcopy, pcopy);
			g_value_array_append(payloads, &vcopy);
			g_value_unset(&vcopy);
		}
	}

	copy = g_object_new(KMS_TYPE_SDP_MEDIA,
				"payloads", payloads,
				"port", self->priv->port,
				"type", self->priv->type,
				"mode", self->priv->mode,
				"bandwidth", self->priv->bandwidth,
				NULL);
	UNLOCK(self);

	g_value_array_free(payloads);

	return copy;
}

static gboolean
contains_payload(GValueArray *list, KmsSdpPayload *pay) {
	gint i;

	for (i = 0; i < list->n_values; i++) {
		KmsSdpPayload *o_pay;

		o_pay = g_value_get_object(g_value_array_get_nth(list, i));

		if (kms_sdp_payload_equals(pay, o_pay))
			return TRUE;
	}

	return FALSE;
}

static KmsSdpMode
inverse_mode(KmsSdpMode mode) {
	switch (mode) {
	case KMS_SDP_MODE_INACTIVE:
		return KMS_SDP_MODE_INACTIVE;
	case KMS_SDP_MODE_RECVONLY:
		return KMS_SDP_MODE_SENDONLY;
	case KMS_SDP_MODE_SENDONLY:
		return KMS_SDP_MODE_RECVONLY;
	case KMS_SDP_MODE_SENDRECV:
		return KMS_SDP_MODE_SENDRECV;
	}

	return KMS_SDP_MODE_INACTIVE;
}

void
kms_sdp_media_intersect(KmsSdpMedia *answerer, KmsSdpMedia *offerer,
				KmsSdpMedia **neg_ans, KmsSdpMedia **neg_off) {
	GValueArray *ans_payloads, *off_payloads;
	KmsSdpMode mode, a_mode, o_mode;
	glong bandwidth, a_bandwidth, o_bandwidth;
	gint i;

	g_return_if_fail(KMS_IS_SDP_MEDIA(answerer));
	g_return_if_fail(KMS_IS_SDP_MEDIA(offerer));
	if (answerer->priv->type != offerer->priv->type)
		return;

	ans_payloads = g_value_array_new(answerer->priv->payloads->n_values);
	off_payloads = g_value_array_new(offerer->priv->payloads->n_values);

	for (i = 0; i < offerer->priv->payloads->n_values; i++) {
		KmsSdpPayload *o_pay;

		o_pay = g_value_get_object(g_value_array_get_nth(
						offerer->priv->payloads, i));

		if (contains_payload(answerer->priv->payloads, o_pay)) {
			GValue aux = G_VALUE_INIT;

			g_value_init(&aux, KMS_TYPE_SDP_PAYLOAD);
			g_value_take_object(&aux, kms_sdp_payload_copy(o_pay));
			g_value_array_append(ans_payloads, &aux);
			g_value_reset(&aux);
			g_value_take_object(&aux, kms_sdp_payload_copy(o_pay));
			g_value_array_append(off_payloads, &aux);
			g_value_unset(&aux);
		}
	}

	if (ans_payloads->n_values == 0)
		mode = KMS_SDP_MODE_INACTIVE;

	a_mode = answerer->priv->mode;
	o_mode = offerer->priv->mode;
	if (a_mode == KMS_SDP_MODE_INACTIVE ||
				o_mode == KMS_SDP_MODE_INACTIVE ||
				(a_mode == KMS_SDP_MODE_RECVONLY &&
					o_mode == KMS_SDP_MODE_RECVONLY) ||
				(a_mode == KMS_SDP_MODE_SENDONLY &&
					o_mode == KMS_SDP_MODE_SENDONLY)) {
		mode = KMS_SDP_MODE_INACTIVE;
	} else if (a_mode == KMS_SDP_MODE_SENDONLY ||
					o_mode == KMS_SDP_MODE_RECVONLY) {
		mode = KMS_SDP_MODE_SENDONLY;
	} else if (a_mode == KMS_SDP_MODE_RECVONLY ||
					o_mode == KMS_SDP_MODE_SENDONLY) {
		mode = KMS_SDP_MODE_RECVONLY;
	} else {
		mode = KMS_SDP_MODE_SENDRECV;
	}

	a_bandwidth = answerer->priv->bandwidth;
	o_bandwidth = offerer->priv->bandwidth;
	if (a_bandwidth == -1)
		bandwidth = o_bandwidth;
	else if (o_bandwidth == -1)
		bandwidth = a_bandwidth;
	else
		bandwidth = a_bandwidth < o_bandwidth ? a_bandwidth : o_bandwidth;


	*neg_ans = g_object_new(KMS_TYPE_SDP_MEDIA,
				"payloads", ans_payloads,
				"port", answerer->priv->port,
				"type", answerer->priv->type,
				"mode", mode,
				"bandwidth", bandwidth,
				NULL);

	*neg_off = g_object_new(KMS_TYPE_SDP_MEDIA,
				"payloads", off_payloads,
				"port", offerer->priv->port,
				"type", answerer->priv->type,
				"mode", inverse_mode(mode),
				"bandwidth", bandwidth,
				NULL);

	g_value_array_free(ans_payloads);
	g_value_array_free(off_payloads);
}

void
kms_sdp_media_dispose(GObject *object) {
	LOCK(object);
	free_payloads(KMS_SDP_MEDIA(object));

	dispose_session(KMS_SDP_MEDIA(object));
	UNLOCK(object);
	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_sdp_media_parent_class)->dispose(object);
}

void
kms_sdp_media_finalize(GObject *object) {
	KmsSdpMedia *self = KMS_SDP_MEDIA(object);

	g_static_mutex_free(&(self->priv->mutex));
	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_sdp_media_parent_class)->finalize(object);
}

static void
kms_sdp_media_class_init(KmsSdpMediaClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec, *payload;

	g_type_class_add_private(klass, sizeof (KmsSdpMediaPriv));

	gobject_class->set_property = kms_sdp_media_set_property;
	gobject_class->get_property = kms_sdp_media_get_property;
	gobject_class->dispose = kms_sdp_media_dispose;
	gobject_class->finalize = kms_sdp_media_finalize;

	pspec = g_param_spec_int("port", "Port",
					"The listening port for media",
					0, G_MAXINT, 0,
					G_PARAM_CONSTRUCT |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_PORT, pspec);

	pspec = g_param_spec_enum("mode", "Connection mode",
				"The connection mode for this media type",
				KMS_SDP_MODE, KMS_SDP_MODE_INACTIVE,
				G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_MODE, pspec);

	pspec = g_param_spec_enum("type", "Media Type",
				"The connection media type",
				KMS_MEDIA_TYPE, KMS_MEDIA_TYPE_UNKNOWN,
				G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_TYPE, pspec);

	pspec = g_param_spec_long("bandwidth", "Bandwidth",
				"The media type bandwidth",
				-1L, G_MAXLONG, 0,
				G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_BANDWIDTH, pspec);

	payload = g_param_spec_object("payload", "Payload",
					"A supported media format",
					KMS_TYPE_SDP_PAYLOAD,
					G_PARAM_READWRITE);

	pspec = g_param_spec_value_array("payloads", "Payloads",
					"Supported media format descriptions",
					payload, G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_PAYLOADS, pspec);

	pspec = g_param_spec_object("session", "Session",
					"The session this media belongs to",
					KMS_TYPE_SDP_SESSION,
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_SESSION, pspec);
}

static void
kms_sdp_media_init(KmsSdpMedia *self) {
	self->priv = KMS_SDP_MEDIA_GET_PRIVATE (self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->type = KMS_MEDIA_TYPE_UNKNOWN;
	self->priv->mode = KMS_SDP_MODE_INACTIVE;
	self->priv->port = 0;
	self->priv->bandwidth = -1;
	self->priv->payloads = NULL;
	self->priv->session = NULL;
}
