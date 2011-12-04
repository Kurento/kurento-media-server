#include "endpoint.h"
#include <glib.h>

#define KMS_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_ENDPOINT, KmsEndpointPriv))

struct _KmsEndpointPriv {
	gchar *localname;
};

enum {
	PROP_0,

	PROP_LOCALNAME
};

G_DEFINE_TYPE(KmsEndpoint, kms_endpoint, G_TYPE_OBJECT);

void
free_localname(KmsEndpoint *self) {
	if (self->priv->localname != NULL) {
		g_free(self->priv->localname);
		self->priv->localname = NULL;
	}
}

void
endpoint_set_property(GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsEndpoint *self = KMS_ENDPOINT(object);

	switch (property_id) {
	case PROP_0:
		/* Do nothing */
		break;
	case PROP_LOCALNAME:
		free_localname(self);
		self->priv->localname = g_value_dup_string(value);
		break;
	default:
		/* We don't have any other property... */
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

void
endpoint_get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsEndpoint *self = KMS_ENDPOINT(object);

	switch (property_id) {
		case PROP_LOCALNAME:
			g_value_set_static_string(value, self->priv->localname);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
kms_endpoint_class_init (KmsEndpointClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private (klass, sizeof (KmsEndpointPriv));

	gobject_class->set_property = endpoint_set_property;
	gobject_class->get_property = endpoint_get_property;

	pspec = g_param_spec_string("localname", "Endpoint local name",
					"Gets the endpoint local name",
					"", G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);
	g_object_class_install_property(gobject_class, PROP_LOCALNAME, pspec);
}

static void
kms_endpoint_init (KmsEndpoint *self) {
	self->priv = KMS_ENDPOINT_GET_PRIVATE (self);
	self->priv->localname = NULL;
}

static void
kms_endpoint_finalyze(KmsEndpoint *self) {
	free_localname(self);
}
