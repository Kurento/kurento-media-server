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

G_LOCK_DEFINE_STATIC(rtmp_receiver_lock);

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

	g_print("Remove rtmp for element: %s\n", GST_OBJECT_NAME(self));
	LOCK(self);
	if (self->priv->finished)
		goto end;

	rtmpsrc = gst_bin_get_by_name(GST_BIN(self), "rtmp");
	if (rtmpsrc == NULL)
		goto end;

	gst_bin_remove(GST_BIN(self), rtmpsrc);
	G_LOCK(rtmp_receiver_lock);
	gst_element_set_state(rtmpsrc, GST_STATE_NULL);
	G_UNLOCK(rtmp_receiver_lock);
	g_object_unref(rtmpsrc);

	rtmpsrc = generate_rtmpsrc(self);
	gst_bin_add(GST_BIN(self), rtmpsrc);
	G_LOCK(rtmp_receiver_lock);
	gst_element_set_state(rtmpsrc, GST_STATE_PLAYING);
	G_UNLOCK(rtmp_receiver_lock);

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
		g_timeout_add_full(G_PRIORITY_DEFAULT, 3000,
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

static void
add_decoder(KmsRtmpReceiver *self, GstElement *prev_tee, GstElement *deco,
							KmsMediaType type) {
	GstElement *queue, *tee;
	GstPad *deco_src;

	queue = kms_utils_create_queue(NULL);
	tee = gst_element_factory_make("tee", NULL);

	if (queue == NULL || tee == NULL) {
		g_warn_if_reached();

		if (queue != NULL)
			g_object_unref(queue);

		if (tee != NULL)
			g_object_unref(tee);

		g_object_unref(deco);
		return;
	}

	gst_element_set_state(deco, GST_STATE_PLAYING);
	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), queue, deco, tee, NULL);

	kms_dynamic_connection(prev_tee, queue, "src");
	kms_dynamic_connection(queue, deco, "src");
	kms_dynamic_connection_tee(deco, tee);

	deco_src = gst_element_get_static_pad(deco, "src");

	if (deco_src == NULL)
		return;

	kms_media_handler_src_set_raw_pad(KMS_MEDIA_HANDLER_SRC(self), deco_src,
					  tee, type);
}

static void
demux_added(GstElement *flvdemux, GstPad *pad, KmsRtmpReceiver *self) {
	GstElement *deco, *tee, *queue, *fakesink;
	GstCaps *caps;
	KmsMediaType type;
	GstPad *tee_sink;

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

	kms_media_handler_src_set_pad(KMS_MEDIA_HANDLER_SRC(self),
						gst_object_ref(pad), tee, type);

	tee_sink = gst_element_get_static_pad(tee, "sink");
	gst_pad_link(pad, tee_sink);
	g_object_unref(tee_sink);

	caps = gst_pad_get_caps(pad);
	deco = kms_utils_get_element_for_caps(GST_ELEMENT_FACTORY_TYPE_DECODER,
					GST_RANK_NONE, caps, GST_PAD_SINK,
					FALSE, NULL);

	gst_caps_unref(caps);

	if (deco == NULL) {
		g_warn_if_reached();
		return;
	}

	deco = prepare_decoder(deco, type);
	add_decoder(self, tee, deco, type);
}

static GstElement*
generate_rtmpsrc(KmsRtmpReceiver *self) {
	GstElement *rtmpsrc;
	gchar *url;

	url = kms_rtmp_session_get_url(self->priv->neg_spec,
				       !self->priv->offerer);
	if (url == NULL) {
		g_warn_if_reached();
		return NULL;
	}

	rtmpsrc = gst_element_factory_make("rtmpsrc", "rtmp");
	if  (rtmpsrc != NULL) {
		g_object_set(rtmpsrc, "location", url, NULL);
		g_object_set(rtmpsrc, "do-timestamp", TRUE, NULL);
		g_object_set(rtmpsrc, "blocksize", 10, NULL);
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
		g_warn_if_reached();

		if (rtmpsrc != NULL)
			g_object_unref(rtmpsrc);
		else
			g_printerr("rtmpsrc should not be null\n");

		if (flvdemux != NULL)
			g_object_unref(flvdemux);
		else
			g_printerr("flvdemux should not be null\n");
		return;
	}

	g_object_connect(flvdemux, "signal::pad_added", demux_added, self, NULL);

	gst_bin_add_many(GST_BIN(self), rtmpsrc, flvdemux, NULL);
	gst_element_link(rtmpsrc, flvdemux);

	/* Because rtmp lib has a problem, this code should not be executed at
	 * the same time from different threads */
	G_LOCK(rtmp_receiver_lock);
	gst_element_set_state(rtmpsrc, GST_STATE_PLAYING);
	G_UNLOCK(rtmp_receiver_lock);
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
