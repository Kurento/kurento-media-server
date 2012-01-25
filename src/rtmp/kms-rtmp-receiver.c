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
	gboolean finished;
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

static GstElement *generate_rtmpsrc(KmsRtmpReceiver *self);

static gboolean
remove_rtmp(KmsRtmpReceiver *self) {
	GstElement *rtmpsrc, *flvdemux;

	LOCK(self);
	if (self->priv->finished)
		goto end;

	rtmpsrc = gst_bin_get_by_name(GST_BIN(self), "rtmp");
	if (rtmpsrc == NULL)
		goto end;

	gst_bin_remove(GST_BIN(self), rtmpsrc);
	gst_element_set_state(rtmpsrc, GST_STATE_NULL);
	g_object_unref(rtmpsrc);

	rtmpsrc = generate_rtmpsrc(self);
	gst_bin_add(GST_BIN(self), rtmpsrc);
	gst_element_set_state(rtmpsrc, GST_STATE_PLAYING);

	flvdemux = gst_bin_get_by_name(GST_BIN(self), "demux");

	if (flvdemux == NULL)
		goto end;

	gst_element_link(rtmpsrc, flvdemux);

end:
	UNLOCK(self);
	return FALSE;
}

static gboolean
event_handler(GstPad *pad, GstEvent *event, KmsRtmpReceiver *self) {
	if (!GST_IS_EVENT(event))
		return TRUE;

	if (GST_EVENT_TYPE(event) == GST_EVENT_EOS) {
		g_timeout_add_full(G_PRIORITY_DEFAULT, 500,
						(GSourceFunc) remove_rtmp,
						g_object_ref(self),
						g_object_unref);

		return FALSE;
	}

	return TRUE;
}

static void
connect_rtmp_callbacks(KmsRtmpReceiver *self, GstElement *rtmpsrc) {
	GstPad *src;

	src = gst_element_get_static_pad(rtmpsrc, "src");

	if (src == NULL)
		return;

	gst_pad_add_event_probe_full(src, G_CALLBACK(event_handler),
					g_object_ref(self), g_object_unref);

	g_object_unref(src);
}

static gboolean
remove_tf_bin(GstElement *elem) {
	GstObject *bin;

	bin = gst_element_get_parent(elem);
	if (bin == NULL)
		goto end;

	gst_element_set_state(GST_ELEMENT(elem), GST_STATE_NULL);
	gst_bin_remove(GST_BIN(bin), GST_ELEMENT(elem));
	g_object_unref(bin);

end:
	g_object_unref(elem);

	return FALSE;
}

static GstElement*
prepare_decoder(GstElement *deco, KmsMediaType type) {
	GstElement *new_deco;
	GstCaps *raw_caps;

	kms_utils_configure_element(deco);

	switch (type) {
		case KMS_MEDIA_TYPE_AUDIO:
			raw_caps = gst_caps_from_string(AUDIO_RAW_CAPS);
			break;
		case KMS_MEDIA_TYPE_VIDEO:
			raw_caps = gst_caps_from_string(VIDEO_RAW_CAPS);
			break;
		default:
			return deco;
	}

	new_deco = kms_generate_bin_with_caps(deco, NULL, raw_caps);
	gst_caps_unref(raw_caps);

	return new_deco;
}

static GstElement*
prepare_identity(GstElement *identity, GstCaps *enc_caps, GstCaps *raw_caps) {
	GstCaps *new_enc_caps;
	GstElement *new_depay;

	new_enc_caps = gst_caps_copy(enc_caps);
	kms_utils_transfer_caps(raw_caps, new_enc_caps);

	new_depay = kms_generate_bin_with_caps(identity, NULL, new_enc_caps);

	gst_caps_unref(new_enc_caps);
	return new_depay;
}

static void
add_decoder(KmsRtmpReceiver *self, GstElement *deco) {
	g_print("TODO: add decoder to element\n");
}

static void
found_raw(GstElement* tf, guint probability, GstCaps* caps,
							KmsRtmpReceiver *self) {
	GstElementFactory *deco_fact;
	GstElement *deco, *peer_elem, *new_deco, *identity, *tee;
	GstObject *bin;
	GstPad *bin_sink, *peer, *pad;
	GstCaps *enc_caps;
	KmsMediaType type;

	KMS_DEBUG;

	bin = gst_element_get_parent(tf);
	type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(bin),
						 MEDIA_TYPE_DATA));

	deco = gst_bin_get_by_name(GST_BIN(bin), "deco");
	deco_fact = gst_element_get_factory(deco);

	bin_sink = gst_element_get_pad(GST_ELEMENT(bin), "sink");
	if (!gst_pad_is_linked(bin_sink))
		goto end;

	/* Disconnect bin and connect new types */
	peer = gst_pad_get_peer(bin_sink);
	enc_caps = gst_caps_copy(GST_PAD_CAPS(bin_sink));
	peer_elem = gst_pad_get_parent_element(peer);

	new_deco = gst_element_factory_create(deco_fact, NULL);
	new_deco = prepare_decoder(new_deco, type);

	identity = gst_element_factory_make("identity", NULL);
	identity = prepare_identity(identity, enc_caps, caps);

	tee = gst_element_factory_make("tee", NULL);

	gst_element_set_state(identity, GST_STATE_PLAYING);
	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(new_deco, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), identity, tee, NULL);

	kms_dynamic_connection(peer_elem, identity, "src");
	kms_dynamic_connection_tee(identity, tee);

	add_decoder(self, new_deco);

	gst_pad_unlink(peer, bin_sink);
	g_object_unref(peer);

	pad = gst_element_get_static_pad(identity, "src");
	kms_media_handler_src_set_pad(KMS_MEDIA_HANDLER_SRC(self), pad, tee,
									type);

	g_object_unref(peer_elem);
	gst_caps_unref(enc_caps);
end:
	g_object_unref(bin_sink);

	/*
	 * HACK: Bin has to be removed once this function finishes to avoid
	 * memory leak or segmentation fault in typefind element
	 */
	g_timeout_add(100, (GSourceFunc) remove_tf_bin, g_object_ref(bin));
	g_object_unref(deco);
	g_object_unref(bin);
}

static void
demux_added(GstElement *flvdemux, GstPad *pad, KmsRtmpReceiver *self) {
	GstElement *bin, *deco, *typefind, *tee, *queue, *fakesink;
	GstCaps *caps;
	KmsMediaType type;
	GstPad *deco_sink, *sink, *tee_sink;

	type = kms_media_type_from_nick(GST_OBJECT_NAME(pad));

	tee = gst_element_factory_make("tee", NULL);
	queue = kms_utils_create_queue(NULL);
	fakesink = gst_element_factory_make("fakesink", NULL);

	if (tee == NULL || queue == NULL || fakesink == NULL) {
		if (tee != NULL)
			g_object_unref(tee);

		if (queue != NULL)
			g_object_unref(queue);

		if (fakesink != NULL)
			g_object_unref(fakesink);

		return;
	}

	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(fakesink, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), tee, queue, fakesink, NULL);
	gst_element_link_many(tee, queue, fakesink, NULL);

	kms_utils_release_unlinked_pads(tee);

	tee_sink = gst_element_get_static_pad(tee, "sink");
	gst_pad_link(pad, tee_sink);
	g_object_unref(tee_sink);

	bin = gst_bin_new(NULL);
	if (bin == NULL) {
		g_warn_if_reached();
		return;
	}

	g_object_set_data(G_OBJECT(bin), MEDIA_TYPE_DATA, GINT_TO_POINTER(type));

	caps = gst_pad_get_caps(pad);
	deco = kms_utils_get_element_for_caps(
					GST_ELEMENT_FACTORY_TYPE_DECODER,
					GST_RANK_NONE, caps, GST_PAD_SINK,
					FALSE, "deco");
	gst_caps_unref(caps);

	if (deco == NULL) {
		g_warn_if_reached();
		g_object_unref(bin);
		return;
	}

	gst_element_set_state(bin, GST_STATE_PLAYING);
	gst_element_set_state(deco, GST_STATE_PLAYING);
	gst_bin_add_many(GST_BIN(bin), deco, NULL);

	typefind = gst_element_factory_make("typefind", "raw_find");
	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(bin), typefind);

	g_object_connect(typefind, "signal::have_type", found_raw, self, NULL);

	gst_element_link_many(deco, typefind, NULL);

	deco_sink = gst_element_get_pad(deco, "sink");
	sink = gst_ghost_pad_new("sink", deco_sink);
	gst_pad_set_active(sink, TRUE);
	gst_element_add_pad(bin, sink);
	g_object_unref(deco_sink);

	gst_bin_add(GST_BIN(self), bin);

	gst_element_link(tee, bin);
}

static GstElement*
generate_rtmpsrc(KmsRtmpReceiver *self) {
	GstElement *rtmpsrc;
	gchar *url;

	url = kms_rtmp_session_get_url(self->priv->neg_spec,
				       self->priv->offerer);
	if (url == NULL) {
		g_warn_if_reached();
		return NULL;
	}

	rtmpsrc = gst_element_factory_make("rtmpsrc", "rtmp");
	if  (rtmpsrc != NULL) {
		g_object_set(rtmpsrc, "location", url, NULL);
		connect_rtmp_callbacks(self, rtmpsrc);
	}

	g_free(url);

	return rtmpsrc;
}

static void
create_media_chain(KmsRtmpReceiver *self) {
	GstElement *rtmpsrc, *flvdemux;

	rtmpsrc = generate_rtmpsrc(self);
	flvdemux = gst_element_factory_make("flvdemux", "demux");

	if (rtmpsrc == NULL || flvdemux == NULL) {
		if (rtmpsrc != NULL)
			g_object_unref(rtmpsrc);

		if (flvdemux != NULL)
			g_object_unref(flvdemux);
		return;
	}

	g_object_connect(flvdemux, "signal::pad_added", demux_added, self, NULL);

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
	self->priv->finished = TRUE;
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
	self->priv->finished = FALSE;
}
