#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"

#define KMS_RTP_RECEIVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_RECEIVER, KmsRtpReceiverPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTP_RECEIVER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTP_RECEIVER(obj)->priv->mutex))

struct _KmsRtpReceiverPriv {
	GMutex *mutex;

	KmsSdpMedia *local_spec;

	GstElement *udpsrc;

	gint audio_port;
	gint video_port;
	gint audio_fd;
	gint video_fd;
};

enum {
	PROP_0,

	PROP_LOCAL_SPEC,
	PROP_AUDIO_FD,
	PROP_VIDEO_FD,
};

G_DEFINE_TYPE(KmsRtpReceiver, kms_rtp_receiver, KMS_TYPE_MEDIA_HANDLER_SRC)

static void
dispose_local_spec(KmsRtpReceiver *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
}

void
kms_rtp_receiver_terminate(KmsRtpReceiver *self) {
	LOCK(self);
	if (self->priv->udpsrc) {
		gst_element_send_event(self->priv->udpsrc,
						gst_event_new_flush_start());
		self->priv->udpsrc = NULL;
	}
	UNLOCK(self);
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			dispose_local_spec(self);
			self->priv->local_spec = g_value_dup_object(value);
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
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->local_spec);
			UNLOCK(self);
			break;
		case PROP_AUDIO_FD:
			LOCK(self);
			g_value_set_int(value, self->priv->audio_fd);
			UNLOCK(self);
			break;
		case PROP_VIDEO_FD:
			LOCK(self);
			g_value_set_int(value, self->priv->video_fd);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static GSList*
get_pay_list(KmsSdpMedia *media){
	GSList *list = NULL;
	GValueArray *payloads;
	gint i;

	g_object_get(media, "payloads", &payloads, NULL);

	for (i = 0; i < payloads->n_values; i++) {
		KmsSdpPayload *aux;

		aux = g_value_get_object(g_value_array_get_nth(payloads, i));
		list = g_slist_prepend(list, aux);
	}
	g_value_array_free(payloads);

	return list;
}

static gint
compare_pay_pt(gconstpointer ppay, gconstpointer ppt) {
	KmsSdpPayload *pay = (gpointer) ppay;
	guint pt;

	g_object_get(pay, "payload", &pt, NULL);

	return pt - (*(int *)ppt);
}

static GstCaps*
get_caps_for_pt(KmsRtpReceiver *self, guint pt) {
	GValueArray *medias;
	GSList *payloads = NULL;
	GstCaps *caps = NULL;
	GSList *l;
	gint i;

	LOCK(self);
	g_object_get(self->priv->local_spec, "medias", &medias, NULL);
	for (i = 0; i < medias->n_values; i++) {
		KmsSdpMedia *aux;
		KmsMediaType type;

		aux = g_value_get_object(g_value_array_get_nth(medias, i));
		g_object_get(aux, "type", &type, NULL);
		switch (type) {
		case KMS_MEDIA_TYPE_AUDIO:
			payloads = g_slist_concat(payloads, get_pay_list(aux));
			break;
		case KMS_MEDIA_TYPE_VIDEO:
			payloads = g_slist_concat(payloads, get_pay_list(aux));
			break;
		default:
			/* No action */
			break;
		}
	}

	l = g_slist_find_custom(payloads, &pt, compare_pay_pt);

	if (l != NULL)
		caps = kms_sdp_payload_to_caps(l->data);

	UNLOCK(self);

	g_value_array_free(medias);
	g_slist_free(payloads);
	return caps;
}

static GstCaps*
request_pt_map(GstElement *demux, guint pt, gpointer self) {
	GstCaps *caps;

	caps = get_caps_for_pt(self, pt);

	if (caps != NULL && gst_caps_is_fixed(caps)) {
		return caps;
	}

	if (caps != NULL)
		gst_caps_unref(caps);

	return NULL;
}

static void
found_type(GstElement* tf, guint probability, GstCaps* caps, gpointer data) {
	GstElement *sink;

	g_print("Found type\n");
	sink = gst_element_factory_make("fakesink", NULL);
	gst_element_set_state(sink, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(data), sink);
	gst_element_link(tf, sink);
}

static void
connect_depay_chain(KmsRtpReceiver *self, GstElement *orig, GstCaps *caps) {
	GstElement *depay, *typefind;

	depay = kms_utils_get_element_for_caps(
					GST_ELEMENT_FACTORY_TYPE_DEPAYLOADER,
					GST_RANK_NONE, caps, GST_PAD_SINK,
					FALSE, NULL);
	g_return_if_fail(depay != NULL);
	gst_element_set_state(depay, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), depay);
	kms_dynamic_connection(orig, depay, "src");

	/* TODO: Connect typefind and notify parent of the available pads */
	typefind = gst_element_factory_make("typefind", NULL);
	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), typefind);
	gst_element_link(depay, typefind);

	g_object_connect(typefind, "signal::have_type", found_type, self, NULL);

}

static void
new_payload_type(GstElement *demux, guint pt, GstPad *pad, gpointer user_data) {
	GstCaps *caps;
	gint len, i;
	gboolean has_clockrate = FALSE;
	GstElement *tee, *sink, *fake_queue, *queue, *buffer;

	tee = gst_element_factory_make("tee", NULL);
	fake_queue = gst_element_factory_make("queue2", NULL);
	sink = gst_element_factory_make("fakesink", NULL);

	g_return_if_fail(tee != NULL && sink != NULL && fake_queue != NULL);

	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(sink, GST_STATE_PLAYING);
	gst_element_set_state(fake_queue, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(user_data), tee, fake_queue, sink, NULL);
	gst_element_link_many(demux, tee, fake_queue, sink, NULL);

	caps = GST_PAD_CAPS(pad);
	len = gst_caps_get_size(caps);
	for (i = 0; i < len; i++) {
		GstStructure *st;
		st = gst_caps_get_structure(caps, 0);

		has_clockrate = gst_structure_has_field(st, "clock-rate");
		if (has_clockrate)
			break;
	}

	if (!has_clockrate)
		goto end;

	queue = gst_element_factory_make("queue2", NULL);
	g_return_if_fail(queue != NULL);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(user_data), queue);
	kms_dynamic_connection(tee, queue, "src");

	buffer = gst_element_factory_make("gstrtpjitterbuffer", NULL);
	g_return_if_fail(buffer != NULL);
	gst_element_set_state(buffer, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(user_data), buffer);
	kms_dynamic_connection(queue, buffer, "src");

	connect_depay_chain(user_data, buffer, caps);

end:
	KMS_DEBUG_PIPE("new_type");
}

static void
create_media_src(KmsRtpReceiver *self, KmsMediaType type) {
	GstElement *udpsrc, *ptdemux;
	GstCaps *caps;
	gchar *udpsrc_name;
	gint *port, *fd;

	switch (type) {
	case KMS_MEDIA_TYPE_AUDIO:
		udpsrc_name = "udpsrc_audio";
		port = &(self->priv->audio_port);
		fd = &(self->priv->audio_fd);
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		udpsrc_name = "udpsrc_video";
		port = &(self->priv->video_port);
		fd = &(self->priv->video_fd);
		break;
	default:
		return;
	}

	caps = gst_caps_from_string("application/x-rtp");
	udpsrc = gst_element_factory_make("udpsrc", udpsrc_name);
	g_object_set(udpsrc, "port", 0, NULL);
	g_object_set(udpsrc, "caps", caps, NULL);
	gst_caps_unref(caps);

	ptdemux = gst_element_factory_make("gstrtpptdemux", NULL);

	gst_bin_add_many(GST_BIN(self), udpsrc, ptdemux, NULL);
	gst_element_link(udpsrc, ptdemux);

	gst_element_set_state(udpsrc, GST_STATE_PLAYING);
	gst_element_set_state(ptdemux, GST_STATE_PLAYING);

	g_object_get(udpsrc, "port", port, "sock", fd, NULL);

	self->priv->udpsrc = udpsrc;

	g_object_connect(ptdemux, "signal::request-pt-map", request_pt_map,
								self, NULL);
	g_object_connect(ptdemux, "signal::new-payload-type", new_payload_type,
								self, NULL);
}

static void
constructed(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);
	GValueArray *medias;
	gint i;

	G_OBJECT_CLASS(kms_rtp_receiver_parent_class)->constructed(object);

	create_media_src(self, KMS_MEDIA_TYPE_AUDIO);
	create_media_src(self, KMS_MEDIA_TYPE_VIDEO);

	g_object_get(self->priv->local_spec, "medias", &medias, NULL);

	for (i = 0; i < medias->n_values; i++) {
		GValue *val;
		KmsSdpMedia *media;
		KmsMediaType type;

		val = g_value_array_get_nth(medias, i);
		media = g_value_get_object(val);

		g_object_get(media, "type", &type, NULL);
		switch (type) {
		case KMS_MEDIA_TYPE_AUDIO:
			g_object_set(media, "port", self->priv->audio_port, NULL);
			break;
		case KMS_MEDIA_TYPE_VIDEO:
			g_object_set(media, "port", self->priv->video_port, NULL);
			break;
		default:
			break;
		}
	}

	g_value_array_free(medias);
}

static void
dispose(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	kms_rtp_receiver_terminate(self);
	LOCK(self);
	dispose_local_spec(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_receiver_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	g_mutex_free(self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_receiver_parent_class)->finalize(object);
}

static void
kms_rtp_receiver_class_init(KmsRtpReceiverClass *klass) {
	GParamSpec *pspec;
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsRtpReceiverPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->constructed = constructed;

	pspec = g_param_spec_object("local-spec", "Local Session Spec",
					"Local Session Spec",
					KMS_TYPE_SDP_SESSION,
					G_PARAM_CONSTRUCT |
					G_PARAM_READWRITE);

	g_object_class_install_property(object_class, PROP_LOCAL_SPEC, pspec);

	pspec = g_param_spec_int("audio-fd", "Audio fd",
					"File descriptor used to send audio",
					-1, G_MAXINT, -1, G_PARAM_READABLE);

	g_object_class_install_property(object_class, PROP_AUDIO_FD, pspec);

	pspec = g_param_spec_int("video-fd", "Video fd",
					"File descriptor used to send video",
					-1, G_MAXINT, -1, G_PARAM_READABLE);

	g_object_class_install_property(object_class, PROP_VIDEO_FD, pspec);
}

static void
kms_rtp_receiver_init(KmsRtpReceiver *self) {
	self->priv = KMS_RTP_RECEIVER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->local_spec = NULL;
}
