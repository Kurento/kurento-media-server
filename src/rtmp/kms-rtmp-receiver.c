#include <kms-core.h>
#include <rtmp/kms-rtmp-receiver.h>
#include <rtmp/kms-rtmp-session.h>
#include "internal/kms-utils.h"

#define KMS_RTMP_RECEIVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTMP_RECEIVER, KmsRtmpReceiverPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTMP_RECEIVER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTMP_RECEIVER(obj)->priv->mutex))

#define MEDIA_TYPE_DATA "type"

struct _KmsRtmpReceiverPriv {
	GMutex *mutex;

	KmsRtmpSession *neg_spec;
	gboolean offerer;
};

enum {
	PROP_0,

	PROP_NEG_SPEC,
	PROP_OFFERER,
};

G_DEFINE_TYPE(KmsRtmpReceiver, kms_rtmp_receiver, KMS_TYPE_MEDIA_HANDLER_SRC)

static void
dispose_neg_spec(KmsRtmpReceiver *self) {
	if (self->priv->neg_spec != NULL) {
		g_object_unref(self->priv->neg_spec);
		self->priv->neg_spec = NULL;
	}
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtmpReceiver *self = KMS_RTMP_RECEIVER(object);

	switch (property_id) {
		case PROP_NEG_SPEC:
			LOCK(self);
			dispose_neg_spec(self);
			self->priv->neg_spec = g_value_dup_object(value);
			UNLOCK(self);
			break;
		case PROP_OFFERER:
			LOCK(self);
			self->priv->offerer = g_value_get_boolean(value);
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
	KmsRtmpReceiver *self = KMS_RTMP_RECEIVER(object);

	switch (property_id) {
		case PROP_NEG_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->neg_spec);
			UNLOCK(self);
			break;
		case PROP_OFFERER:
			LOCK(self);
			g_value_set_boolean(value, self->priv->offerer);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
connect_rtmp_callbacks(GstElement *rtmpsrc) {
	KMS_LOG_DEBUG("TODO: Add callbacks to rtmpsrc to handle eos");
}

static void
demux_added(GstElement *flvdemux, GstPad *pad, KmsRtmpReceiver *self) {
	KMS_LOG_DEBUG("TODO: Connect new pad to flvdemux");
}

static void
create_media_chain(KmsRtmpReceiver *self) {
	GstElement *rtmpsrc, *flvdemux;
	gchar *url;

	rtmpsrc = gst_element_factory_make("rtmpsrc", NULL);
	flvdemux = gst_element_factory_make("flvdemux", NULL);

	url = kms_rtmp_session_get_url(self->priv->neg_spec,
							self->priv->offerer);
	if (url == NULL) {
		g_warn_if_reached();
		return;
	}

	if (rtmpsrc == NULL || flvdemux == NULL) {
		if (rtmpsrc != NULL)
			g_object_unref(rtmpsrc);

		if (flvdemux != NULL)
			g_object_unref(flvdemux);
		return;
	}

	g_object_set(rtmpsrc, "location", url, NULL);
	g_free(url);

	g_object_connect(flvdemux, "signal::pad_added", demux_added, self, NULL);
	connect_rtmp_callbacks(rtmpsrc);

	gst_bin_add_many(GST_BIN(self), rtmpsrc, flvdemux, NULL);
	gst_element_link(rtmpsrc, flvdemux);

	gst_element_set_state(rtmpsrc, GST_STATE_PLAYING);
	gst_element_set_state(flvdemux, GST_STATE_PLAYING);
}

static void
constructed(GObject *object) {
	KmsRtmpReceiver *self = KMS_RTMP_RECEIVER(object);

	G_OBJECT_CLASS(kms_rtmp_receiver_parent_class)->constructed(object);

	g_return_if_fail(self->priv->neg_spec != NULL);

	create_media_chain(self);
}

static void
dispose(GObject *object) {
	KmsRtmpReceiver *self = KMS_RTMP_RECEIVER(object);

	LOCK(self);
	dispose_neg_spec(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtmp_receiver_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtmpReceiver *self = KMS_RTMP_RECEIVER(object);

	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtmp_receiver_parent_class)->finalize(object);
}

static void
kms_rtmp_receiver_class_init(KmsRtmpReceiverClass *klass) {
	GParamSpec *pspec;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsRtmpReceiverPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->constructed = constructed;

	pspec = g_param_spec_object("neg-spec", "Negotiated Session Spec",
				    "Negotiated Session Description",
			     KMS_TYPE_RTMP_SESSION,
			     G_PARAM_CONSTRUCT_ONLY |
			     G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_NEG_SPEC, pspec);

	pspec = g_param_spec_boolean("offerer", "Offerer",
				     "If local party is offerer",
			      FALSE, G_PARAM_CONSTRUCT_ONLY |
			      G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_OFFERER, pspec);

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_rtmp_receiver_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_rtmp_receiver_parent_class)->numpadtemplates;
}

static void
kms_rtmp_receiver_init(KmsRtmpReceiver *self) {
	self->priv = KMS_RTMP_RECEIVER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->neg_spec = NULL;
	self->priv->offerer = FALSE;
}
