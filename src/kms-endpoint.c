#include "kms-endpoint.h"
#include <glib.h>

#define KMS_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_ENDPOINT, KmsEndpointPriv))

struct _KmsEndpointPriv {
	gchar *localname;
	gulong local_count;
	GStaticMutex local_count_mutex;
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

KmsConnection *
kms_endpoint_create_connection(KmsEndpoint *self, KmsConnectionType type,
								GError **err) {
	gchar *name;

	g_static_mutex_lock(&(self->priv->local_count_mutex));
	name = g_strdup_printf("%s-%d", self->priv->local_count++);
	g_static_mutex_unlock(&(self->priv->local_count_mutex));

	switch(type) {
	case KMS_CONNECTION_TYPE_LOCAL:
		break;

	case KMS_CONNECTION_TYPE_RTP:
		return KMS_ENDPOINT_GET_CLASS(self)->create_connection(self,
								name, err);
		break;
	}
}

static KmsConnection *
default_create_connection(KmsEndpoint *self, gchar* name, GError **err) {
	gchar *msg = g_strdup_printf("Class %s does not reimplement "
				"create_connection method",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));

	g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, msg);

	if (*err == NULL) {
		*err = g_error_new(KMS_ENDPOINT_ERROR,
			KMS_ENDPOINT_ERROR_NOT_IMPLEMENTED, msg);
	}

	g_free(msg);

	return NULL;
}

static void
kms_endpoint_finalize(GObject *gobject) {
	KmsEndpoint *self = KMS_ENDPOINT(gobject);

	free_localname(self);
	g_static_mutex_free(&(self->priv->local_count_mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS (kms_endpoint_parent_class)->finalize(gobject);
}

static void
kms_endpoint_class_init (KmsEndpointClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private (klass, sizeof (KmsEndpointPriv));

	gobject_class->set_property = endpoint_set_property;
	gobject_class->get_property = endpoint_get_property;
	gobject_class->finalize = kms_endpoint_finalize;

	pspec = g_param_spec_string("localname", "Endpoint local name",
					"Gets the endpoint local name",
					"", G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_LOCALNAME, pspec);

	/* Set default implementation to avoid segment violation errors */
	klass->create_connection = default_create_connection;
}

static void
kms_endpoint_init (KmsEndpoint *self) {
	self->priv = KMS_ENDPOINT_GET_PRIVATE (self);

	self->priv->localname = NULL;
	self->priv->local_count = 0;
	g_static_mutex_init(&(self->priv->local_count_mutex));
}
