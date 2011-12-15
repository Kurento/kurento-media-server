#include "kms-endpoint.h"
#include "kms-media-handler-manager.h"
#include "kms-local-connection.h"
#include <glib.h>

#define KMS_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_ENDPOINT, KmsEndpointPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_ENDPOINT(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_ENDPOINT(obj)->priv->mutex)))

struct _KmsEndpointPriv {
	gchar *localname;
	gulong local_count;
	GStaticMutex mutex;
	KmsMediaHandlerManager *manager;
	KmsConnection *connection;
	GSList *connections;
};

enum {
	PROP_0,

	PROP_LOCALNAME,
	PROP_MANAGER
};

G_DEFINE_TYPE(KmsEndpoint, kms_endpoint, G_TYPE_OBJECT)

static void
free_localname(KmsEndpoint *self) {
	if (self->priv->localname != NULL) {
		g_free(self->priv->localname);
		self->priv->localname = NULL;
	}
}

static void
dispose_manager(KmsEndpoint *self) {
	if (self->priv->manager != NULL) {
		g_object_unref(self->priv->manager);
		self->priv->manager = NULL;
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
		LOCK(self);
		free_localname(self);
		self->priv->localname = g_value_dup_string(value);
		UNLOCK(self);
		break;
	case PROP_MANAGER: {
		gpointer pmanager = g_value_get_pointer(value);
		LOCK(self);
		dispose_manager(self);
		if (KMS_IS_MEDIA_HANDLER_MANAGER(pmanager))
			self->priv->manager = g_object_ref(
					KMS_MEDIA_HANDLER_MANAGER(pmanager));
		else
			self->priv->manager = NULL;
		UNLOCK(self);
		break;
	}
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
			LOCK(self);
			g_value_set_string(value, self->priv->localname);
			UNLOCK(self);
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
	KmsConnection *conn = NULL;

	LOCK(self);
	name = g_strdup_printf("%s-%ld", self->priv->localname,
						self->priv->local_count++);
	UNLOCK(self);

	switch(type) {
	case KMS_CONNECTION_TYPE_LOCAL: {
		GSList *l;

		if (self->priv->manager == NULL) {
			if (err != NULL && *err == NULL)
				*err = g_error_new(KMS_ENDPOINT_ERROR,
					KMS_ENDPOINT_ERROR_ALREADY_CREATED,
					"Local connection can not be created "
					"because manager has not been set for "
					"%s.", G_OBJECT_CLASS_NAME(
						G_OBJECT_GET_CLASS(self)));
			return NULL;
		}

		/* TODO: Assing a connection manager to handle media */
		conn = g_object_new(KMS_TYPE_LOCAL_CONNECTION, "id", name,
							"endpoint", self, NULL);
		LOCK(self);
		l = self->priv->connections;
		self->priv->connections = g_slist_prepend(l, g_object_ref(conn));
		UNLOCK(self);
		goto end;
	}
	case KMS_CONNECTION_TYPE_RTP: {
		if (self->priv->connection != NULL) {
			if (err != NULL && *err == NULL)
				*err = g_error_new(KMS_ENDPOINT_ERROR,
					KMS_ENDPOINT_ERROR_ALREADY_CREATED,
					"Only one remote connection per enpoint"
					" is supported.");
			return NULL;
		}
		conn = KMS_ENDPOINT_GET_CLASS(self)->create_connection(self,
								name, err);
		if (conn != NULL)
			self->priv->connection = g_object_ref(conn);
		goto end;
	}
	}

end:
	g_free(name);
	return conn;
}

gboolean
kms_endpoint_delete_connection(KmsEndpoint *self, KmsConnection *conn,
								GError **err) {
	GSList *l;
	gboolean ret;

	LOCK(self);
	l = g_slist_find(self->priv->connections, conn);
	if (l == NULL && self->priv->connection != conn) {
		UNLOCK(self);
		if (err != NULL && *err == NULL)
			*err = g_error_new(KMS_ENDPOINT_ERROR,
					KMS_ENDPOINT_ERROR_NOT_FOUND,
					"Connection is not available on this "
					"endpoint.");
		return FALSE;
	}

	if (self->priv->connection == conn)
		self->priv->connection = NULL;
	else
		self->priv->connections = g_slist_delete_link(
						self->priv->connections, l);
	UNLOCK(self);

	/* TODO: notify the connection deletion with a signal*/
	ret = kms_connection_terminate(conn, err);

	g_object_unref(conn);
	return ret;
}

static KmsConnection *
default_create_connection(KmsEndpoint *self, gchar* name, GError **err) {
	gchar *msg = g_strdup_printf("Class %s does not reimplement "
				"create_connection method",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));

	g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, msg);

	if (err != NULL && *err == NULL) {
		*err = g_error_new(KMS_ENDPOINT_ERROR,
			KMS_ENDPOINT_ERROR_NOT_IMPLEMENTED, msg);
	}

	g_free(msg);

	return NULL;
}

static void
kms_endpoint_dispose(GObject *gobject) {
	KmsEndpoint *self = KMS_ENDPOINT(gobject);

	LOCK(self);
	dispose_manager(self);

	if (self->priv->connection != NULL) {
		g_object_unref(self->priv->connection);
		self->priv->connection = NULL;
	}
	UNLOCK(self);

	/* TODO: delete all connections */

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_endpoint_parent_class)->dispose(gobject);
}

static void
kms_endpoint_finalize(GObject *gobject) {
	KmsEndpoint *self = KMS_ENDPOINT(gobject);

	free_localname(self);
	g_static_mutex_free(&(self->priv->mutex));

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
	gobject_class->dispose = kms_endpoint_dispose;
	gobject_class->finalize = kms_endpoint_finalize;

	pspec = g_param_spec_string("localname", "Endpoint local name",
					"Gets the endpoint local name",
					"", G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_LOCALNAME, pspec);

	pspec = g_param_spec_pointer("manager", "The media handler manager",
					"Media handler manager that will "
					"provide a media handler factory",
					G_PARAM_WRITABLE);

	g_object_class_install_property(gobject_class, PROP_MANAGER, pspec);

	/* Set default implementation to avoid segment violation errors */
	klass->create_connection = default_create_connection;
}

static void
kms_endpoint_init (KmsEndpoint *self) {
	self->priv = KMS_ENDPOINT_GET_PRIVATE (self);

	self->priv->localname = NULL;
	self->priv->local_count = 0;
	g_static_mutex_init(&(self->priv->mutex));
	self->priv->manager = NULL;
	self->priv->connection = NULL;
	self->priv->connections = NULL;
}
