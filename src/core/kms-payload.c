#include "kms-payload.h"

#define KMS_PAYLOAD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_PAYLOAD, KmsPayloadPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_PAYLOAD(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_PAYLOAD(obj)->priv->mutex)))

struct _KmsPayloadPriv {
	GStaticMutex mutex;
	gchar *name;
	gint clockrate;
	gint payload;
	/*
	 * KmsMediaSpec *mediaSpec;
	 */

};

enum {
	PROP_0,

	PROP_PAYLOAD,
	PROP_NAME,
	PROP_CLOCKRATE
};

G_DEFINE_TYPE(KmsPayload, kms_payload, G_TYPE_OBJECT)

static void
free_name(KmsPayload *self) {
	if (self->priv->name != NULL) {
		g_free(self->priv->name);
		self->priv->name = NULL;
	}
}

static void
kms_payload_set_property(GObject  *object, guint property_id,
				const GValue *value, GParamSpec *pspec) {
	KmsPayload *self = KMS_PAYLOAD(object);

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
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_payload_get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsPayload *self = KMS_PAYLOAD(object);

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
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

void
kms_payload_dispose(GObject *object) {
}

void
kms_payload_finalize(GObject *object) {
	KmsPayload *self = KMS_PAYLOAD(object);

	free_name(self);
	g_static_mutex_free(&(self->priv->mutex));
}

static void
kms_payload_class_init(KmsPayloadClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private(klass, sizeof (KmsPayloadPriv));

	gobject_class->set_property = kms_payload_set_property;
	gobject_class->get_property = kms_payload_get_property;
	gobject_class->dispose = kms_payload_dispose;
	gobject_class->finalize = kms_payload_finalize;

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
}

static void
kms_payload_init(KmsPayload *self) {
	self->priv = KMS_PAYLOAD_GET_PRIVATE (self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->name = NULL;
	self->priv->clockrate = 0;
	self->priv->payload = 0;
}
