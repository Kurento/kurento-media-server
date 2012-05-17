/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <rtmp/kms-rtmp-endpoint.h>
#include <rtmp/kms-rtmp-connection.h>
#include <kms_session_spec_types.h>

#define KMS_RTMP_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTMP_ENDPOINT, KmsRtmpEndpointPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTMP_ENDPOINT(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTMP_ENDPOINT(obj)->priv->mutex)))

struct _KmsRtmpEndpointPriv {
	GStaticMutex mutex;
	KmsSessionSpec *local_spec;
};

enum {
	PROP_0,

	PROP_LOCAL_SPEC
};

G_DEFINE_TYPE(KmsRtmpEndpoint, kms_rtmp_endpoint, KMS_TYPE_ENDPOINT)

static void
dispose_local_spec(KmsRtmpEndpoint *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
}

static KmsConnection*
create_connection(KmsEndpoint *object, gchar *name, GError **err) {
	KmsRtmpConnection *conn;
	KmsRtmpEndpoint *self = KMS_RTMP_ENDPOINT(object);
	KmsSessionSpec *local_spec;

	LOCK(self);
	if (self->priv->local_spec == NULL) {
		/* generate an default spec */
		self->priv->local_spec = g_object_new(KMS_TYPE_SESSION_SPEC,
									NULL);
	}

	local_spec = kms_session_spec_copy(self->priv->local_spec);
	conn = g_object_new(KMS_TYPE_RTMP_CONNECTION, "id", name,
					"endpoint", self,
					"local-spec", local_spec,
					NULL);
	g_object_unref(local_spec);

	UNLOCK(self);
	g_object_set(G_OBJECT(self), "manager", conn, NULL);
	return KMS_CONNECTION(conn);
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtmpEndpoint *self = KMS_RTMP_ENDPOINT(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			dispose_local_spec(self);
			self->priv->local_spec = kms_session_spec_filter_transport(
						g_value_get_object(value),
						KMS_TYPE_TRANSPORT_RTMP);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsRtmpEndpoint *self = KMS_RTMP_ENDPOINT(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->local_spec);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
dispose(GObject *object) {
	KmsRtmpEndpoint *self = KMS_RTMP_ENDPOINT(object);

	LOCK(self);
	dispose_local_spec(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtmp_endpoint_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtmpEndpoint *self = KMS_RTMP_ENDPOINT(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtmp_endpoint_parent_class)->finalize(object);
}

static void
kms_rtmp_endpoint_class_init(KmsRtmpEndpointClass *klass) {
	GParamSpec *pspec;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsRtmpEndpointPriv));

	KMS_ENDPOINT_CLASS(klass)->create_connection = create_connection;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;

	pspec = g_param_spec_object("local-spec", "Local Session Spec",
							"Local Session Spec",
							KMS_TYPE_SESSION_SPEC,
							G_PARAM_CONSTRUCT |
							G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_LOCAL_SPEC, pspec);
}

static void
kms_rtmp_endpoint_init(KmsRtmpEndpoint *self) {
	self->priv = KMS_RTMP_ENDPOINT_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->local_spec = NULL;
}
