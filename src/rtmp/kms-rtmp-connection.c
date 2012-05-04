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

#include <kms-core.h>
#include <rtmp/kms-rtmp-connection.h>
#include <rtmp/kms-rtmp-sender.h>
#include <rtmp/kms-rtmp-receiver.h>
#include <rtmp/kms-rtmp-session.h>
#include "internal/kms-utils.h"
#include <uuid/uuid.h>

#define KMS_RTMP_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTMP_CONNECTION, KmsRtmpConnectionPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_RTMP_CONNECTION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_RTMP_CONNECTION(obj)->priv->mutex)))

enum {
	PROP_0,

	PROP_LOCAL_SPEC,
	PROP_DESCRIPTOR,
};

struct _KmsRtmpConnectionPriv {
	GStaticMutex mutex;
	KmsRtmpSession *local_spec;
	KmsRtmpSession *remote_spec;
	KmsRtmpSession *negotiated;
	KmsRtmpSession *descriptor;
	KmsRtmpReceiver *receiver;
	KmsRtmpSender *sender;
	gboolean initialized;
	gboolean offerer;
};

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);
static void media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsRtmpConnection, kms_rtmp_connection,
				KMS_TYPE_CONNECTION,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init)
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_FACTORY,
					media_handler_factory_iface_init))

static void
dispose_receiver(KmsRtmpConnection *self) {
	if (self->priv->receiver != NULL) {
		kms_media_handler_src_terminate(
				KMS_MEDIA_HANDLER_SRC(self->priv->receiver));
		g_object_unref(self->priv->receiver);
		self->priv->receiver = NULL;
	}
}

static void
dispose_sender(KmsRtmpConnection *self) {
	if (self->priv->sender != NULL) {
		kms_media_handler_sink_terminate(
				KMS_MEDIA_HANDLER_SINK(self->priv->sender));
		g_object_unref(self->priv->sender);
		self->priv->sender = NULL;
	}
}

static void
dispose_local_spec(KmsRtmpConnection *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
}

static void
dispose_remote_spec(KmsRtmpConnection *self) {
	if (self->priv->remote_spec != NULL) {
		g_object_unref(self->priv->remote_spec);
		self->priv->remote_spec = NULL;
	}
}

static void
dispose_negotiated(KmsRtmpConnection *self) {
	if (self->priv->negotiated != NULL) {
		g_object_unref(self->priv->negotiated);
		self->priv->negotiated = NULL;
	}
}

static void
dispose_descriptor(KmsRtmpConnection *self) {
	if (self->priv->descriptor != NULL) {
		g_object_unref(self->priv->descriptor);
		self->priv->descriptor = NULL;
	}
}

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	return KMS_MEDIA_HANDLER_FACTORY(g_object_ref(iface));
}

static void
dispose_factory(KmsMediaHandlerManager *manager,  KmsMediaHandlerFactory *factory) {
	/* Nothing to do, just avoid the warning */
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
	iface->dispose_factory = dispose_factory;
}

static KmsMediaHandlerSrc*
get_src(KmsMediaHandlerFactory *iface) {
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(iface);
	KmsMediaHandlerSrc *src;

	LOCK(self);
	if (self->priv->receiver != NULL)
		src = KMS_MEDIA_HANDLER_SRC(g_object_ref(self->priv->receiver));
	else
		src = NULL;
	UNLOCK(self);

	return src;
}

static KmsMediaHandlerSink*
get_sink(KmsMediaHandlerFactory *iface) {
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(iface);
	KmsMediaHandlerSink *sink;

	LOCK(self);
	if (self->priv->sender != NULL)
		sink = KMS_MEDIA_HANDLER_SINK(g_object_ref(self->priv->sender));
	else
		sink = NULL;
	UNLOCK(self);

	return sink;
}

static void
media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface) {
	iface->get_sink = get_sink;
	iface->get_src = get_src;
}

static void
create_rtmp_sender(KmsRtmpConnection *self) {
	self->priv->sender = g_object_new(KMS_TYPE_RTMP_SENDER,
					"neg-spec", self->priv->negotiated,
					"offerer", self->priv->offerer,
					NULL);
}

static void
create_rtmp_receiver(KmsRtmpConnection *self) {
	self->priv->receiver = g_object_new(KMS_TYPE_RTMP_RECEIVER,
					"neg-spec", self->priv->negotiated,
					"offerer", self->priv->offerer,
					NULL);
}

static gboolean
connect_to_remote(KmsConnection *conn, KmsSdpSession *spec, GError **err) {
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(conn);

	if (!KMS_IS_SDP_SESSION(spec)) {
		SET_ERROR(err, KMS_RTMP_CONNECTION_ERROR,
					KMS_RTMP_CONNECTION_ERROR_WRONG_VALUE,
					"Given spect has incorrect type");
		return FALSE;
	}

	LOCK(self);
	if (self->priv->remote_spec != NULL) {
		SET_ERROR(err, KMS_RTMP_CONNECTION_ERROR,
					KMS_RTMP_CONNECTION_ERROR_ALREADY,
					"Remote spec was already set");
		UNLOCK(self);
		return FALSE;
	}

	self->priv->remote_spec = kms_rtmp_session_create_from_sdp_session(spec);

	if (self->priv->initialized) {
		self->priv->offerer = TRUE;
		self->priv->negotiated = kms_rtmp_session_intersect(
							self->priv->remote_spec,
							self->priv->local_spec);
	} else {
		self->priv->negotiated = kms_rtmp_session_intersect(
							self->priv->local_spec,
							self->priv->remote_spec);
	}

	dispose_descriptor(self);
	self->priv->descriptor = g_object_ref(self->priv->negotiated);

	/* Create rtmpsender */
	create_rtmp_sender(self);
	/* Create rtmpsender */
	create_rtmp_receiver(self);

	UNLOCK(self);

	return TRUE;
}

static gboolean
mode_changed(KmsConnection *self, KmsConnectionMode mode, KmsMediaType type,
								GError **err) {
	/* TODO: Implement mode changed for RtmpConnection */
	return TRUE;
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			dispose_local_spec(self);
			self->priv->local_spec = g_value_dup_object(value);
			if (self->priv->descriptor == NULL &&
					self->priv->local_spec != NULL) {
				self->priv->descriptor = g_object_ref(
							self->priv->local_spec);
			}
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
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(object);
	KmsSdpSession *sdp_session;

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->local_spec);
			UNLOCK(self);
			break;
		case PROP_DESCRIPTOR:
			LOCK(self);
			sdp_session = kms_rtmp_session_get_sdp_session(
							self->priv->descriptor);
			g_value_set_object(value, sdp_session);
			if (sdp_session != NULL)
				g_object_unref(sdp_session);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static gchar*
generate_unique_identifier() {
	uuid_t uuid;
	gchar *uuid_str;

	uuid_str = g_malloc(sizeof(gchar) * 37);
	uuid_generate(uuid);
	uuid_unparse(uuid, uuid_str);

	return uuid_str;
}

static void
constructed(GObject *object) {
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(object);
	gchar *offerer;

	G_OBJECT_CLASS(kms_rtmp_connection_parent_class)->constructed(object);

	if (kms_get_pipeline() == NULL) {
		g_warning("Kms should be initialized before instantiate "
								"objects");
		g_assert_not_reached();
		return;
	}

	g_return_if_fail(self->priv->local_spec != NULL);

	offerer = generate_unique_identifier();
	g_object_set(self->priv->local_spec, "offerer", offerer, NULL);
	g_free(offerer);

	self->priv->descriptor = kms_rtmp_session_copy(self->priv->local_spec);
}

static void
kms_rtmp_connection_dispose(GObject *object) {
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(object);

	/* TODO: Unset manager on endpoint */

	LOCK(self);
	dispose_local_spec(self);
	dispose_remote_spec(self);
	dispose_negotiated(self);
	dispose_descriptor(self);
	dispose_receiver(self);
	dispose_sender(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtmp_connection_parent_class)->dispose(object);
}

static void
kms_rtmp_connection_finalize(GObject *object) {
	KmsRtmpConnection *self = KMS_RTMP_CONNECTION(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtmp_connection_parent_class)->finalize(object);
}

static void
kms_rtmp_connection_class_init (KmsRtmpConnectionClass *klass) {
	GParamSpec *pspec;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private (klass, sizeof (KmsRtmpConnectionPriv));

	KMS_CONNECTION_CLASS(klass)->mode_changed = mode_changed;
	KMS_CONNECTION_CLASS(klass)->connect_to_remote = connect_to_remote;
	gobject_class->dispose = kms_rtmp_connection_dispose;
	gobject_class->finalize = kms_rtmp_connection_finalize;
	gobject_class->constructed = constructed;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;

	pspec = g_param_spec_object("local-spec", "Local Session Spec",
					"Local Session Spec",
					KMS_TYPE_RTMP_SESSION,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_LOCAL_SPEC, pspec);

	pspec = g_param_spec_object("descriptor", "Session descriptor",
					"The current session descriptor",
					KMS_TYPE_SDP_SESSION,
					G_PARAM_READABLE);

	g_object_class_install_property(gobject_class, PROP_DESCRIPTOR, pspec);
}

static void
kms_rtmp_connection_init (KmsRtmpConnection *self) {
	self->priv = KMS_RTMP_CONNECTION_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));
	self->priv->local_spec = NULL;
	self->priv->descriptor = NULL;
	self->priv->receiver = NULL;
	self->priv->sender = NULL;
	self->priv->remote_spec = NULL;
	self->priv->initialized = FALSE;
	self->priv->negotiated = NULL;
}
