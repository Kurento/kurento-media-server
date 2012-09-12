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
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"
#include <nice.h>

#define KMS_RTP_RECEIVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_RTP_RECEIVER, KmsRtpReceiverPriv))

#define LOCK(obj) (g_mutex_lock(KMS_RTP_RECEIVER(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_RTP_RECEIVER(obj)->priv->mutex))

#define MEDIA_TYPE_DATA "type"
#define STREAM_ID_DATA "stream_id"

static void start_media(KmsRtpReceiver *self);

struct _KmsRtpReceiverPriv {
	GMutex *mutex;

	KmsSessionSpec *local_spec;

	NiceAgent *audio_agent;
	NiceAgent *video_agent;

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
	PROP_AUDIO_AGENT,
	PROP_VIDEO_AGENT,
};

G_DEFINE_TYPE(KmsRtpReceiver, kms_rtp_receiver, KMS_TYPE_MEDIA_HANDLER_SRC)

static void
dispose_video_agent(KmsRtpReceiver *self) {
	if (self->priv->video_agent != NULL) {
		g_object_unref(self->priv->video_agent);
		self->priv->video_agent = NULL;
	}
}

static void
dispose_audio_agent(KmsRtpReceiver *self) {
	if (self->priv->audio_agent != NULL) {
		g_object_unref(self->priv->audio_agent);
		self->priv->audio_agent = NULL;
	}
}

static void
dispose_local_spec(KmsRtpReceiver *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
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
			start_media(self);
			UNLOCK(self);
			break;
		case PROP_AUDIO_AGENT:
			LOCK(self);
			dispose_audio_agent(self);
			self->priv->audio_agent = g_value_dup_object(value);
			UNLOCK(self);
			break;
		case PROP_VIDEO_AGENT:
			LOCK(self);
			dispose_video_agent(self);
			self->priv->video_agent = g_value_dup_object(value);
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
get_pay_list(KmsMediaSpec *media){
	GSList *list = NULL;
	gint i;

	for (i = 0; i < media->payloads->len; i++) {
		KmsPayload *aux;

		aux = media->payloads->pdata[i];
		list = g_slist_prepend(list, aux);
	}

	return list;
}

static gint
compare_pay_pt(gconstpointer ppay, gconstpointer ppt) {
	KmsPayload *pay = (gpointer) ppay;

	if (!pay->__isset_rtp || pay->rtp == NULL)
		return -1;

	return pay->rtp->id - (*(int *)ppt);
}

static GstCaps*
get_caps_for_pt(KmsRtpReceiver *self, guint pt) {
	KmsSessionSpec *spec = self->priv->local_spec;
	GstCaps *caps = NULL;
	gint i;

	LOCK(self);
	for (i = 0; i < spec->medias->len; i++) {
		KmsMediaSpec *media;

		media = spec->medias->pdata[i];

		if (g_hash_table_size(media->type) == 1) {
			GSList *payloads, *l;

			payloads = get_pay_list(media);

			l = g_slist_find_custom(payloads, &pt, compare_pay_pt);

			if (l != NULL) {
				caps = kms_payload_to_caps(l->data, media);
			}

			g_slist_free(payloads);

			if (caps != NULL)
				goto end;
		}
	}

end:
	UNLOCK(self);
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
prepare_depay(GstElement *depay, GstCaps *rtp_caps, GstCaps *enc_caps,
							GstCaps *raw_caps) {
	GstCaps *new_enc_caps;
	GstElement *new_depay;

	kms_utils_configure_element(depay);

	new_enc_caps = gst_caps_copy(enc_caps);
	kms_utils_transfer_caps(raw_caps, new_enc_caps);
	kms_utils_transfer_caps(rtp_caps, new_enc_caps);

	new_depay = kms_generate_bin_with_caps(depay, NULL, new_enc_caps);

	gst_caps_unref(new_enc_caps);
	return new_depay;
}

static void
add_elements_to_bin(KmsRtpReceiver *self, GstElement *orig, GstElement *deco,
					GstElement *depay, KmsMediaType type) {
	GstElement *deco_tee, *depay_tee, *queue;
	GstPad *deco_src, *depay_src;

	deco_tee = gst_element_factory_make("tee", NULL);
	depay_tee = gst_element_factory_make("tee", NULL);
	queue = kms_utils_create_queue(NULL);
	if (deco_tee == NULL || depay_tee == NULL || queue == NULL) {
		if (depay_tee != NULL)
			g_object_unref(depay_tee);

		if (deco_tee != NULL)
			g_object_unref(deco_tee);

		if (queue != NULL)
			g_object_unref(queue);

		g_object_unref(deco);
		g_object_unref(depay);
		return;
	}

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(deco, GST_STATE_PLAYING);
	gst_element_set_state(depay, GST_STATE_PLAYING);
	gst_element_set_state(deco_tee, GST_STATE_PLAYING);
	gst_element_set_state(depay_tee, GST_STATE_PLAYING);

	/* TODO: Add a queue between deco and depay_tee */

	gst_bin_add_many(GST_BIN(self), queue, depay, depay_tee, deco,
								deco_tee, NULL);
	kms_dynamic_connection(orig, queue, "src");
	kms_dynamic_connection(queue, depay, "src");
	kms_dynamic_connection_tee(depay, depay_tee);
	kms_dynamic_connection(depay_tee, deco, "src");
	kms_dynamic_connection_tee(deco, deco_tee);

	depay_src = gst_element_get_static_pad(depay, "src");
	if (depay_src == NULL)
		return;

	kms_media_handler_src_set_pad(KMS_MEDIA_HANDLER_SRC(self), depay_src,
							depay_tee, type);

	deco_src = gst_element_get_static_pad(deco, "src");

	if (deco_src == NULL)
		return;

	kms_media_handler_src_set_raw_pad(KMS_MEDIA_HANDLER_SRC(self), deco_src,
							deco_tee, type);
}

static void
found_raw(GstElement* tf, guint probability, GstCaps* caps,
							KmsRtpReceiver *self) {
	GstElement *deco, *depay, *new_deco, *new_depay, *peer_elem;
	GstElement *enc_find, *raw_find;
	GstElementFactory *deco_fact, *depay_fact;
	GstObject *bin;
	GstPad *bin_sink, *peer;
	KmsMediaType type;
	GstCaps *enc_caps, *raw_caps, *rtp_caps;

	bin = gst_element_get_parent(tf);
	type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(bin),
							MEDIA_TYPE_DATA));

	deco = gst_bin_get_by_name(GST_BIN(bin), "deco");
	depay = gst_bin_get_by_name(GST_BIN(bin), "depay");
	enc_find = gst_bin_get_by_name(GST_BIN(bin), "enc_find");
	raw_find = gst_bin_get_by_name(GST_BIN(bin), "raw_find");

	g_object_get(enc_find, "caps", &enc_caps, NULL);
	g_object_get(raw_find, "caps", &raw_caps, NULL);
	g_object_unref(enc_find);
	g_object_unref(raw_find);

	deco_fact = gst_element_get_factory(deco);
	depay_fact = gst_element_get_factory(depay);

	bin_sink = gst_element_get_pad(GST_ELEMENT(bin), "sink");
	if (!gst_pad_is_linked(bin_sink))
		goto end;

	/* Disconnect bin and connect new types */
	peer = gst_pad_get_peer(bin_sink);
	gst_pad_unlink(peer, bin_sink);
	rtp_caps = gst_caps_copy(GST_PAD_CAPS(bin_sink));

	peer_elem = gst_pad_get_parent_element(peer);
	g_object_unref(peer);

	new_deco = gst_element_factory_create(deco_fact, NULL);
	new_depay = gst_element_factory_create(depay_fact, NULL);

	new_deco = prepare_decoder(new_deco, type);
	new_depay = prepare_depay(new_depay, rtp_caps, enc_caps, raw_caps);

	type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(bin),
							MEDIA_TYPE_DATA));

	add_elements_to_bin(self, peer_elem, new_deco, new_depay, type);
	g_object_unref(peer_elem);

	gst_caps_unref(rtp_caps);
end:
	g_object_unref(bin_sink);
	gst_caps_unref(enc_caps);
	gst_caps_unref(raw_caps);

	/*
	 * HACK: Bin has to be removed once this function finishes to avoid
	 * memory leak or segmentation fault in typefind element
	 */
	g_timeout_add(100, (GSourceFunc) remove_tf_bin, g_object_ref(bin));
	g_object_unref(deco);
	g_object_unref(depay);
	g_object_unref(bin);
}

static void
found_coded(GstElement* tf, guint probability, GstCaps* caps,
							KmsRtpReceiver *self) {
	GstElement *deco, *typefind;
	GstObject *bin;

	bin = gst_element_get_parent(tf);
	deco = kms_utils_get_element_for_caps(
					GST_ELEMENT_FACTORY_TYPE_DECODER,
					GST_RANK_NONE, caps, GST_PAD_SINK,
					FALSE, "deco");
	if (deco == NULL) {
		g_warn_if_reached();
		g_object_unref(bin);
		gst_bin_remove(GST_BIN(self), GST_ELEMENT(bin));
		return;
	}

	gst_element_set_state(deco, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(bin), deco);

	typefind = gst_element_factory_make("typefind", "raw_find");
	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(bin), typefind);

	g_object_connect(typefind, "signal::have_type", found_raw, self, NULL);

	gst_element_link_many(tf, deco, typefind, NULL);
	g_object_unref(bin);
}

static void
connect_depay_chain(KmsRtpReceiver *self, GstElement *orig, GstCaps *caps,
							KmsMediaType type) {
	GstElement *bin, *depay, *typefind, *queue;
	GstPad *sink, *queue_sink;

	bin = gst_bin_new(NULL);
	if (bin == NULL) {
		g_warn_if_reached();
		return;
	}
	g_object_set_data(G_OBJECT(bin), MEDIA_TYPE_DATA, GINT_TO_POINTER(type));

	queue = kms_utils_create_queue(NULL);
	if (queue == NULL) {
		g_warn_if_reached();
		g_object_unref(bin);
	}

	depay = kms_utils_get_element_for_caps(
					GST_ELEMENT_FACTORY_TYPE_DEPAYLOADER,
					GST_RANK_NONE, caps, GST_PAD_SINK,
					FALSE, "depay");
	if (depay == NULL) {
		g_warn_if_reached();
		g_object_unref(queue);
		g_object_unref(bin);
		return;
	}

	gst_element_set_state(bin, GST_STATE_PLAYING);
	gst_element_set_state(depay, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_bin_add_many(GST_BIN(bin), queue, depay, NULL);

	typefind = gst_element_factory_make("typefind", "enc_find");
	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(bin), typefind);

	g_object_connect(typefind, "signal::have_type", found_coded, self, NULL);

	gst_element_link_many(queue, depay, typefind, NULL);

	queue_sink = gst_element_get_pad(queue, "sink");
	sink = gst_ghost_pad_new("sink", queue_sink);
	gst_pad_set_active(sink, TRUE);
	gst_element_add_pad(bin, sink);
	g_object_unref(queue_sink);

	gst_bin_add(GST_BIN(self), bin);
	gst_element_link(orig, bin);
}

static void
new_payload_type(GstElement *demux, guint pt, GstPad *pad, gpointer user_data) {
	GstCaps *caps;
	gint len, i;
	gboolean has_clockrate = FALSE;
	GstElement *buffer, *tee, *sink, *fake_queue;
	KmsMediaType type;

	buffer = gst_element_factory_make("gstrtpjitterbuffer", NULL);
	tee = gst_element_factory_make("tee", NULL);
	fake_queue = kms_utils_create_queue(NULL);
	sink = kms_utils_create_fakesink(NULL);

	if (buffer == NULL || tee == NULL || sink == NULL || fake_queue == NULL) {
		g_warn_if_reached();

		if (buffer != NULL)
			g_object_unref(buffer);

		if (tee != NULL)
			g_object_unref(tee);

		if (sink != NULL)
			g_object_unref(sink);

		if (fake_queue != NULL)
			g_object_unref(fake_queue);
	}

	gst_element_set_state(buffer, GST_STATE_PLAYING);
	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(sink, GST_STATE_PLAYING);
	gst_element_set_state(fake_queue, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(user_data), buffer, tee, fake_queue, sink, NULL);
	gst_element_link_many(demux, buffer, tee, fake_queue, sink, NULL);

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

	type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(demux),
							MEDIA_TYPE_DATA));
	connect_depay_chain(user_data, tee, caps, type);

end:
	KMS_DEBUG_PIPE("new_type");
}

static void
create_media_src(KmsRtpReceiver *self, KmsMediaType type) {
	GstElement *src, *ptdemux;
	gchar *src_name, *demux_name;
	gint port, *fd;
	NiceAgent *agent;

	switch (type) {
	case KMS_MEDIA_TYPE_AUDIO:
		src_name = "src_audio";
		demux_name = "demux_audio";
		port = self->priv->audio_port;
		fd = &(self->priv->audio_fd);
		agent = self->priv->audio_agent;
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		src_name = "src_video";
		demux_name = "demux_video";
		port = self->priv->video_port;
		fd = &(self->priv->video_fd);
		agent = self->priv->video_agent;
		break;
	default:
		return;
	}

	if (agent != NULL) {
		guint stream_id;

		src = gst_element_factory_make("nicesrc", src_name);
		stream_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(agent),
							STREAM_ID_DATA));

		g_object_set(src, "agent", agent, "stream", stream_id,
						"component", (guint) 1, NULL);
	} else {
		src = gst_element_factory_make("udpsrc", src_name);
		g_object_set(src, "port", port, NULL);
	}

	ptdemux = gst_element_factory_make("gstrtpptdemux", demux_name);

	gst_bin_add_many(GST_BIN(self), src, ptdemux, NULL);
	gst_element_link(src, ptdemux);

	gst_element_set_state(src, GST_STATE_PLAYING);
	gst_element_set_state(ptdemux, GST_STATE_PLAYING);

	if (agent == NULL) {
		g_object_get(src, "sock", fd, NULL);
	}

	g_object_set_data(G_OBJECT(ptdemux), MEDIA_TYPE_DATA,
							GINT_TO_POINTER(type));
	g_object_connect(ptdemux, "signal::request-pt-map", request_pt_map,
								self, NULL);
	g_object_connect(ptdemux, "signal::new-payload-type", new_payload_type,
								self, NULL);
}

static void
start_media(KmsRtpReceiver *self) {
	KmsSessionSpec *spec = self->priv->local_spec;
	gint i;

	for (i = 0; i < spec->medias->len; i++) {
		KmsMediaSpec *media;

		media = spec->medias->pdata[i];

		if (g_hash_table_lookup(media->type, (gpointer)
			KMS_MEDIA_TYPE_AUDIO)) {
			self->priv->audio_port = media->transport->rtp->port;
			} else if (g_hash_table_lookup(media->type, (gpointer)
				KMS_MEDIA_TYPE_VIDEO)) {
				self->priv->video_port = media->transport->rtp->port;
				}
	}

	create_media_src(self, KMS_MEDIA_TYPE_AUDIO);
	create_media_src(self, KMS_MEDIA_TYPE_VIDEO);
}

static void
constructed(GObject *object) {
	G_OBJECT_CLASS(kms_rtp_receiver_parent_class)->constructed(object);
}

static void
dispose(GObject *object) {
	KmsRtpReceiver *self = KMS_RTP_RECEIVER(object);

	LOCK(self);
	/* TODO: Evaluate the use of flush events */
// 	if (self->priv->udpsrc) {
// 		gst_element_send_event(self->priv->udpsrc,
// 				       gst_event_new_flush_start());
// 		self->priv->udpsrc = NULL;
// 	}
	dispose_local_spec(self);
	dispose_audio_agent(self);
	dispose_video_agent(self);
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
					KMS_TYPE_SESSION_SPEC,
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

	pspec = g_param_spec_object("audio-agent", "ICE Audio Agent",
						"ICE Audio Agent",
						NICE_TYPE_AGENT,
						G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_AUDIO_AGENT, pspec);

	pspec = g_param_spec_object("video-agent", "ICE Video Agent",
						"ICE Video Agent",
						NICE_TYPE_AGENT,
						G_PARAM_WRITABLE);

	g_object_class_install_property(object_class, PROP_VIDEO_AGENT, pspec);

	/* HACK:
		Don't know why but padtemplates are NULL in child classes,
		this hack takes them from parent class
	*/
	GST_ELEMENT_CLASS(klass)->padtemplates =
		GST_ELEMENT_CLASS(kms_rtp_receiver_parent_class)->padtemplates;
	GST_ELEMENT_CLASS(klass)->numpadtemplates =
		GST_ELEMENT_CLASS(kms_rtp_receiver_parent_class)->numpadtemplates;
}

static void
kms_rtp_receiver_init(KmsRtpReceiver *self) {
	self->priv = KMS_RTP_RECEIVER_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->local_spec = NULL;
	self->priv->audio_agent = NULL;
	self->priv->video_agent = NULL;
	self->priv->video_port = 0;
	self->priv->audio_port = 0;
}
