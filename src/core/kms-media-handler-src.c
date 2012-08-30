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
#include "internal/kms-utils.h"

#define KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))

#define GST_CAT_DEFAULT kms_media_handler_src
GST_DEBUG_CATEGORY_STATIC(GST_CAT_DEFAULT);

#define TEE "tee"
#define MEDIA_TYPE "type"
#define NEG_BW "neg_bg"

struct _KmsMediaHandlerSrcPriv {
	GStaticMutex mutex;

	GstPad *audio_prefered_pad;
	GstPad *audio_raw_pad;
	GSList *audio_other_pads;
	GstPad *video_prefered_pad;
	GstPad *video_raw_pad;
	GSList *video_other_pads;
	GstElement *audio_bw_queue;
	GstElement *audio_bw_sink;
	GstElement *video_bw_queue;
	GstElement *video_bw_sink;

	guint audio_bps;
	guint video_bps;

	guint bw_source;

	GSList *encoders;
};

enum {
	PROP_0,

};

static GstStaticPadTemplate audio_src = GST_STATIC_PAD_TEMPLATE (
						"audio_src%d",
						GST_PAD_SRC,
						GST_PAD_REQUEST,
						GST_STATIC_CAPS_ANY
					);

static GstStaticPadTemplate video_src = GST_STATIC_PAD_TEMPLATE (
						"video_src%d",
						GST_PAD_SRC,
						GST_PAD_REQUEST,
						GST_STATIC_CAPS_ANY
					);

G_DEFINE_TYPE(KmsMediaHandlerSrc, kms_media_handler_src, GST_TYPE_BIN)

static void
dispose_internal_element(KmsMediaHandlerSrc *self, GstElement *elem) {
	GstBin *bin = GST_BIN(self);

	g_object_ref(elem);
	gst_bin_remove(bin, elem);
	gst_element_set_state(elem, GST_STATE_NULL);
	g_object_unref(elem);
}

static void
dispose_audio_bw_queue(KmsMediaHandlerSrc *self) {
	if (self->priv->audio_bw_queue != NULL) {
		dispose_internal_element(self, self->priv->audio_bw_queue);
		self->priv->audio_bw_queue = NULL;
	}
}

static void
dispose_audio_bw_sink(KmsMediaHandlerSrc *self) {
	if (self->priv->audio_bw_sink != NULL) {
		dispose_internal_element(self, self->priv->audio_bw_sink);
		self->priv->audio_bw_sink = NULL;
	}
}

static void
dispose_video_bw_queue(KmsMediaHandlerSrc *self) {
	if (self->priv->video_bw_queue != NULL) {
		dispose_internal_element(self, self->priv->video_bw_queue);
		self->priv->video_bw_queue = NULL;
	}
}

static void
dispose_video_bw_sink(KmsMediaHandlerSrc *self) {
	if (self->priv->video_bw_sink != NULL) {
		dispose_internal_element(self, self->priv->video_bw_sink);
		self->priv->video_bw_sink = NULL;
	}
}

static void
dispose_audio_prefered_pad(KmsMediaHandlerSrc *self) {
	if (self->priv->audio_prefered_pad != NULL) {
		g_object_unref(self->priv->audio_prefered_pad);
		self->priv->audio_prefered_pad = NULL;
	}
}

static void
dispose_video_prefered_pad(KmsMediaHandlerSrc *self) {
	if (self->priv->video_prefered_pad != NULL) {
		g_object_unref(self->priv->video_prefered_pad);
		self->priv->video_prefered_pad = NULL;
	}
}

static void
dispose_audio_raw_pad(KmsMediaHandlerSrc *self) {
	if (self->priv->audio_raw_pad != NULL) {
		g_object_unref(self->priv->audio_raw_pad);
		self->priv->audio_raw_pad = NULL;
	}
}

static void
dispose_video_raw_pad(KmsMediaHandlerSrc *self) {
	if (self->priv->video_raw_pad != NULL) {
		g_object_unref(self->priv->video_raw_pad);
		self->priv->video_raw_pad = NULL;
	}
}

static void
dispose_audio_other_pads(KmsMediaHandlerSrc *self) {
	if (self->priv->audio_other_pads != NULL) {
		g_slist_free_full(self->priv->audio_other_pads, g_object_unref);
		self->priv->audio_other_pads = NULL;
	}
}

static void
dispose_video_other_pads(KmsMediaHandlerSrc *self) {
	if (self->priv->video_other_pads != NULL) {
		g_slist_free_full(self->priv->video_other_pads, g_object_unref);
		self->priv->video_other_pads = NULL;
	}
}

void
kms_media_handler_src_terminate(KmsMediaHandlerSrc *self) {
	G_OBJECT_GET_CLASS(self)->dispose(G_OBJECT(self));
}

gboolean
kms_media_handler_src_connect(KmsMediaHandlerSrc *self,
						KmsMediaHandlerSink *sink,
						KmsMediaType type,
						GError **err) {
	GstIterator *it;
	GstPad *pad, *src_pad, *sink_pad;
	gboolean done = FALSE, linked = FALSE;
	gchar *src_name, *sink_name;
	GEnumClass *eclass;
	GEnumValue *evalue;
	gboolean ret = TRUE;
	GstPadLinkReturn link_ret;

	if (sink == NULL)
		return TRUE;

	if (self == NULL)
		return TRUE;

	eclass = G_ENUM_CLASS(g_type_class_ref(KMS_MEDIA_TYPE));
	evalue = g_enum_get_value(eclass, type);
	g_type_class_unref(eclass);

	/* Check if it is already linked otherwise link */
	it = gst_element_iterate_sink_pads(GST_ELEMENT(sink));
	while (!done && !linked) {
		switch (gst_iterator_next(it, (gpointer *)&pad)) {
		case GST_ITERATOR_OK:
			if (g_strstr_len(GST_OBJECT_NAME(pad), -1,
					evalue->value_nick) != NULL &&
						gst_pad_is_linked(pad)) {
				GstPad *peer;
				GstElement *elem;

				peer = gst_pad_get_peer(pad);
				elem = gst_pad_get_parent_element(peer);
				if (elem != NULL) {
					linked = (elem == GST_ELEMENT(self));
					gst_object_unref(elem);
				}
			}
			gst_object_unref(pad);
			break;
		case GST_ITERATOR_RESYNC:
			gst_iterator_resync(it);
			break;
		case GST_ITERATOR_ERROR:
			done = TRUE;
			break;
		case GST_ITERATOR_DONE:
			done = TRUE;
			break;
		}
	}
	gst_iterator_free(it);

	if (linked)
		return TRUE;


	src_name = g_strdup_printf("%s_src%s", evalue->value_nick, "%d");
	sink_name = g_strdup_printf("%s_sink", evalue->value_nick);
	src_pad = gst_element_get_request_pad(GST_ELEMENT(self), src_name);
	if (src_pad == NULL) {
		g_print("Pad %s not found in %s\n", src_name, GST_OBJECT_NAME(self));
		ret = TRUE;
		goto end;
	}
	sink_pad = gst_element_get_static_pad(GST_ELEMENT(sink), sink_name);
	if (sink_pad == NULL) {
		gst_element_release_request_pad(GST_ELEMENT(self), src_pad);
		gst_object_unref(src_pad);
		g_print("Pad %s not found in %s\n", sink_name, GST_OBJECT_NAME(sink));
		ret = TRUE;
		goto end;
	}

	if (gst_pad_is_linked(sink_pad)) {
		GstPad *tmp_src;
		tmp_src = gst_pad_get_peer(sink_pad);
		gst_pad_unlink(tmp_src, sink_pad);
		g_object_unref(tmp_src);
	}

	g_print("linking: %s and %s\n", GST_OBJECT_NAME(self),
							GST_OBJECT_NAME(sink));

	link_ret = gst_pad_link(src_pad, sink_pad);

	eclass = G_ENUM_CLASS(g_type_class_ref(GST_TYPE_PAD_LINK_RETURN));
	evalue = g_enum_get_value(eclass, link_ret);
	g_type_class_unref(eclass);

	if (GST_PAD_LINK_FAILED(link_ret)) {
		g_set_error(err, KMS_MEDIA_HANDLER_SRC_ERROR,
					KMS_MEDIA_HANDLER_SRC_ERROR_LINK_ERROR,
					"Error linking %s and %s: %s",
					GST_OBJECT_NAME(self),
					GST_OBJECT_NAME(sink),
					evalue->value_nick);
		ret = FALSE;
		gst_element_release_request_pad(GST_ELEMENT(self), src_pad);
		gst_object_unref(src_pad);
		g_object_unref(sink_pad);
		goto end;
	}

	g_object_unref(src_pad);
	g_object_unref(sink_pad);
end:

	g_free(sink_name);
	g_free(src_name);

	return ret;
}

static void
pad_unlinked(GstPad  *pad, GstPad  *peer, GstElement *elem) {
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);
	gst_element_release_request_pad(elem, pad);
}

static gboolean
check_pad_compatible(GstPad *pad, GstCaps *caps) {
	GstCaps *pad_caps, *neg_caps = NULL;
	gboolean ret = FALSE;

	if (pad == NULL)
		return FALSE;

	pad_caps = gst_pad_get_caps(pad);

	ret = gst_caps_can_intersect(pad_caps, caps);

	if (!ret)
		goto end;

	/* If there are caps negotiated, check that match with the target caps */
	neg_caps = gst_pad_get_negotiated_caps(pad);
	if (neg_caps == NULL)
		goto end;

	if (gst_caps_can_intersect(neg_caps, caps)) {
		ret = TRUE;
		goto end;
	}

	ret = FALSE;

end:
	gst_caps_unref(pad_caps);
	if (neg_caps)
		gst_caps_unref(neg_caps);

	return ret;
}

static GstPad*
generate_raw_chain_audio(KmsMediaHandlerSrc *self, GstPad *raw,
							gboolean dynamic) {
	GstElement *tee, *queue, *convert, *resample, *rate;
	GstPad *rate_src;

	if (!GST_IS_PAD(raw)) {
		g_warn_if_reached();
		return NULL;
	}

	if (!KMS_IS_MEDIA_HANDLER_SRC(self)) {
		g_warn_if_reached();
		return NULL;
	}

	queue = kms_utils_create_queue(NULL);
	convert = gst_element_factory_make("audioconvert", NULL);
	resample = gst_element_factory_make("audioresample", NULL);
	rate = gst_element_factory_make("audiorate", NULL);
	tee = g_object_get_data(G_OBJECT(raw), TEE);

	if (queue == NULL || convert == NULL || resample == NULL ||
						rate == NULL || tee == NULL) {
		g_warn_if_reached();

		if (queue != NULL)
			g_object_unref(queue);
		else
			g_warning("queue is NULL");

		if (convert != NULL)
			g_object_unref(convert);
		else
			g_warning("convert is NULL");

		if (resample != NULL)
			g_object_unref(resample);
		else
			g_warning("resample is NULL");

		if (rate != NULL)
			g_object_unref(rate);
		else
			g_warning("rate is NULL");

		if (tee == NULL)
			g_warning("tee is NULL");

		return NULL;
	}

	g_object_set(convert, "dithering", 0, NULL);
	g_object_set(resample, "quality", 8, NULL);
	g_object_set(rate, "tolerance", GST_MSECOND * 250,
				"skip-to-first", TRUE, NULL);

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(convert, GST_STATE_PLAYING);
	gst_element_set_state(resample, GST_STATE_PLAYING);
	gst_element_set_state(rate, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), queue, convert, resample, rate, NULL);

	kms_dynamic_connection_full(tee, queue, "src", TRUE);
	kms_dynamic_connection_full(queue, convert, "src", TRUE);
	kms_dynamic_connection_full(convert, resample, "src", TRUE);
	kms_dynamic_connection_full(resample, rate, "src", TRUE);

	rate_src = gst_element_get_static_pad(rate, "src");
	g_object_set_data(G_OBJECT(rate_src), TEE, rate);

	return rate_src;
}

static GstPad*
generate_raw_chain_video(KmsMediaHandlerSrc *self, GstPad *raw,
							gboolean dynamic) {
	GstElement *tee, *queue, *colorspace, *rate, *videoscale;
	GstPad *scale_src;

	queue = kms_utils_create_queue(NULL);
	colorspace = gst_element_factory_make("colorspace", NULL);
	rate = gst_element_factory_make("videomaxrate", NULL);
	videoscale = gst_element_factory_make("videoscale", NULL);
	tee = g_object_get_data(G_OBJECT(raw), TEE);

	if (queue == NULL || colorspace == NULL || rate == NULL ||
					videoscale == NULL || tee == NULL) {
		g_warn_if_reached();

		if (queue != NULL)
			g_object_unref(queue);

		if (colorspace != NULL)
			g_object_unref(colorspace);

		if (rate != NULL)
			g_object_unref(rate);

		if (videoscale != NULL)
			g_object_unref(videoscale);
	}


	/* TODO: Add a filter with following caps
			"video/x-raw-yuv, pixel-aspect-ratio=[0/1, 1/1];"
			"video/x-raw-rgb, pixel-aspect-ratio=[0/1, 1/1];"
			"video/x-raw-gray, pixel-aspect-ratio=[0/1, 1/1]");
	*/

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(colorspace, GST_STATE_PLAYING);
	gst_element_set_state(rate, GST_STATE_PLAYING);
	gst_element_set_state(videoscale, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), queue, colorspace, rate, videoscale,
									NULL);

	kms_dynamic_connection_full(tee, queue, "src", TRUE);
	kms_dynamic_connection_full(queue, colorspace, "src", TRUE);
	kms_dynamic_connection_full(colorspace, rate, "src", TRUE);
	kms_dynamic_connection_full(rate, videoscale, "src", TRUE);

	scale_src = gst_element_get_static_pad(videoscale, "src");
	g_object_set_data(G_OBJECT(scale_src), TEE, videoscale);

	return scale_src;
}

static GstPad*
generate_raw_chain_full(KmsMediaHandlerSrc *self, GstPad *raw,
					KmsMediaType type, gboolean dynamic) {
	if (type == KMS_MEDIA_TYPE_AUDIO)
		return generate_raw_chain_audio(self, raw, dynamic);
	if (type == KMS_MEDIA_TYPE_VIDEO)
		return generate_raw_chain_video(self, raw, dynamic);
	return NULL;
}

static GstPad*
generate_raw_chain(KmsMediaHandlerSrc *self, GstPad *raw, KmsMediaType type) {
	return generate_raw_chain_full(self, raw, type, TRUE);
}

static GstPad*
generate_new_target_pad(KmsMediaHandlerSrc *self, GstPad *raw,
					GstCaps *caps, KmsMediaType type) {
	GstElement *encoder, *orig_elem;
	GstPad *orig_pad = NULL, *target_pad;
	gint bw;

	encoder = kms_utils_get_element_for_caps(
					GST_ELEMENT_FACTORY_TYPE_ENCODER,
					GST_RANK_NONE, caps, GST_PAD_SRC,
					FALSE, NULL);

	if (encoder == NULL)
		return NULL;

	orig_pad = generate_raw_chain_full(self, raw, type, FALSE);
	if (orig_pad == NULL)
		goto error;

	kms_utils_configure_element(encoder);

	bw = kms_utils_get_bandwidth_from_caps(caps);
	bw = bw == - 1 ? 0 : bw * 1000;
	g_object_set_data(G_OBJECT(encoder), NEG_BW, GINT_TO_POINTER(bw));
	g_object_set_data(G_OBJECT(encoder), MEDIA_TYPE, GINT_TO_POINTER(type));
	self->priv->encoders = g_slist_prepend(self->priv->encoders, encoder);

	encoder = kms_generate_bin_with_caps(encoder, NULL, caps);

	orig_elem = g_object_get_data(G_OBJECT(orig_pad), TEE);
	if (orig_elem == NULL)
		goto error;

	gst_element_set_state(encoder, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), encoder);
	kms_dynamic_connection(orig_elem, encoder, "src");

	target_pad = gst_element_get_static_pad(encoder, "src");
	if (target_pad == NULL)
		goto error;

	g_object_set_data(G_OBJECT(target_pad), TEE, encoder);

	g_object_unref(orig_pad);

	return target_pad;

error:
	if (orig_pad != NULL)
		g_object_unref(orig_pad);

	g_object_unref(encoder);

	return NULL;
}

static gboolean
check_raw_caps(KmsMediaType type, GstCaps *caps) {
	GstCaps *raw, *intersect;
	gboolean ret;

	switch (type) {
	case KMS_MEDIA_TYPE_AUDIO:
		raw = gst_caps_from_string(AUDIO_RAW_CAPS);
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		raw = gst_caps_from_string(VIDEO_RAW_CAPS);
		break;
	default:
		return FALSE;
	}

	intersect = gst_caps_intersect(raw, caps);

	ret = !gst_caps_is_empty(intersect);

	gst_caps_unref(intersect);
	gst_caps_unref(raw);
	return ret;
}

static GstPad*
get_target_pad(KmsMediaHandlerSrc *self, GstPad *peer, GstPad *prefered,
						GstPad *raw, GSList **other,
						KmsMediaType type) {
	GstCaps *caps;
	GstPad *new_pad, *target = NULL;
	GSList *l;

	caps = gst_pad_get_caps(peer);

	/* Check prefered pad */
	if (check_pad_compatible(prefered, caps)) {
		target = g_object_ref(prefered);
		goto end;
	}
	/* Check raw pad */
	if (GST_IS_PAD(raw) && check_raw_caps(type, caps)) {
		target = generate_raw_chain(self, raw, type);
		goto end;
	}
	/* Check other pads */
	l = *other;
	while (l != NULL) {
		if (check_pad_compatible(l->data, caps)) {
			target = g_object_ref(l->data);
		}
		l = l->next;
	}

	/* Try to generate a new pad */
	if (raw == NULL)
		goto end;

	new_pad = generate_new_target_pad(self, raw, caps, type);
	if (new_pad == NULL)
		goto end;

	*other = g_slist_append(*other, g_object_ref(new_pad));

	target = new_pad;

end:
	gst_caps_unref(caps);
	return target;
}

static void
set_target_pad(GstGhostPad *gp, GstPad *target) {
	GstElement *tee;

	tee = g_object_get_data(G_OBJECT(target), TEE);
	if (tee == NULL) {
		g_object_unref(target);
		return;
	}

	kms_utils_connect_target_with_queue(tee, gp);
}

static GstPadLinkReturn
link_pad(GstPad *pad, GstPad *peer) {
	KmsMediaType type;
	KmsMediaHandlerSrc *self;
	GstElement *elem;
	GstPad *target_pad = NULL;
	GstPadLinkReturn ret = GST_PAD_LINK_OK;

	type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pad), MEDIA_TYPE));

	elem = gst_pad_get_parent_element(pad);
	if (elem == NULL)
		return GST_PAD_LINK_WRONG_HIERARCHY;
	self = KMS_MEDIA_HANDLER_SRC(elem);

	LOCK(self);
	switch(type) {
	case KMS_MEDIA_TYPE_AUDIO:
		target_pad = get_target_pad(self, peer,
					self->priv->audio_prefered_pad,
					self->priv->audio_raw_pad,
					&(self->priv->audio_other_pads), type);
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		target_pad = get_target_pad(self, peer,
					self->priv->video_prefered_pad,
					self->priv->video_raw_pad,
					&(self->priv->video_other_pads), type);
		break;
	default:
		ret = GST_PAD_LINK_NOFORMAT;
	}

	if (GST_PAD_LINKFUNC(peer) && GST_PAD_LINK_SUCCESSFUL(ret))
		ret = GST_PAD_LINKFUNC(peer)(peer, pad);

	if (target_pad != NULL && GST_PAD_LINK_SUCCESSFUL(ret)) {
		set_target_pad(GST_GHOST_PAD(pad), target_pad);
	}

	if (target_pad != NULL)
		g_object_unref(target_pad);

	UNLOCK(self);
	g_object_unref(elem);
	return ret;
}

static GstPad*
request_new_pad(GstElement *elem, GstPadTemplate *templ, const gchar *name) {
	GstPad *pad;
	gchar *new_name;

	if (name != NULL)
		new_name = g_strdup(name);
	else
		new_name = kms_utils_generate_pad_name(templ->name_template);

	pad = gst_ghost_pad_new_no_target_from_template(new_name, templ);
	g_free(new_name);
	gst_pad_set_active(pad, TRUE);
	if (g_strstr_len(templ->name_template, -1, "audio")) {
		g_object_set_data(G_OBJECT(pad), MEDIA_TYPE,
				  GINT_TO_POINTER(KMS_MEDIA_TYPE_AUDIO));
	} else if (g_strstr_len(templ->name_template, -1, "video")) {
		g_object_set_data(G_OBJECT(pad), MEDIA_TYPE,
				  GINT_TO_POINTER(KMS_MEDIA_TYPE_VIDEO));
	}
	gst_pad_set_link_function(pad, link_pad);
	g_object_connect(pad, "signal::unlinked", pad_unlinked, elem, NULL);

	gst_element_add_pad(elem, pad);
	return pad;
}

static void
connect_pads(KmsMediaHandlerSrc *self) {
	GstIterator *it;
	GstPad *pad, *peer, *target;
	gboolean done = FALSE;

	it = gst_element_iterate_src_pads(GST_ELEMENT(self));

	while (!done) {
		switch (gst_iterator_next(it, (gpointer *)&pad)) {
		case GST_ITERATOR_OK:
			target = gst_ghost_pad_get_target(GST_GHOST_PAD(pad));
			if (target == NULL && gst_pad_is_linked(GST_PAD(pad))) {
				/* TODO: Possible race condition, pad can be
				* 	unlinked during this process */
				peer = gst_pad_get_peer(GST_PAD(pad));
				link_pad(GST_PAD(pad), peer);
				g_object_unref(peer);
			} else if (target != NULL) {
				g_object_unref(target);
			}
			gst_object_unref(pad);
			break;
		case GST_ITERATOR_RESYNC:
			gst_iterator_resync(it);
			break;
		case GST_ITERATOR_ERROR:
			done = TRUE;
			break;
		case GST_ITERATOR_DONE:
			done = TRUE;
			break;
		}
	}
	gst_iterator_free(it);
}

static void
fix_bandwidth(KmsMediaHandlerSrc *self, GstElement *encoder) {
	KmsMediaType type;
	gint neg_bw;
	guint bw;

	neg_bw = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(encoder), NEG_BW));
	type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(encoder), MEDIA_TYPE));

	bw = kms_media_handler_sink_get_bandwidth(self, type);

	kms_utils_configure_bw(encoder, neg_bw, bw);
}

static guint
get_bw(GstElement *queue) {
	if (GST_IS_ELEMENT(queue)) {
		guint bytes;
		guint64 time_ns;
		gdouble time_s;

		g_object_get(queue, "current-level-bytes", &bytes, NULL);
		g_object_get(queue, "current-level-time", &time_ns, NULL);

		time_s = time_ns / (gdouble) G_GUINT64_CONSTANT(1000000000);

		if (time_s > 1.5) {
			glong bps;

			bps = (glong) ((((gdouble) bytes) * 8) / time_s);
			return bps;
		}
	}

	return 0;
}

static gboolean
bw_calculator(gpointer data) {
	KmsMediaHandlerSrc *self = data;
	GstElement *queue;
	GSList *l;

	if (!KMS_IS_MEDIA_HANDLER_SRC(self)) {
		return FALSE;
	}

	LOCK(self);
	queue = self->priv->audio_bw_queue;
	self->priv->audio_bps = get_bw(queue);
	queue = self->priv->video_bw_queue;
	self->priv->video_bps = get_bw(queue);

	for (l = self->priv->encoders; l != NULL; l = l->next) {
		fix_bandwidth(self, l->data);
	}
	UNLOCK(self);
	return TRUE;
}

static void
create_bw_calculator(KmsMediaHandlerSrc *self, KmsMediaType type, GstPad *pad,
							GstElement *tee) {
	GstElement *queue, *fakesink;

	if (type != KMS_MEDIA_TYPE_AUDIO && type != KMS_MEDIA_TYPE_VIDEO)
		return;

	queue = kms_utils_create_queue(NULL);
	fakesink = kms_utils_create_fakesink(NULL);

	if (queue == NULL || fakesink == NULL) {
		if (queue != NULL)
			g_object_unref(queue);

		if (fakesink != NULL)
			g_object_unref(fakesink);
	}

	g_object_set(queue, "max-size-buffers", 0, "max-size-bytes", 0,
		     "max-size-time", G_GUINT64_CONSTANT(2000000000), NULL);
	g_object_set(fakesink, "ts-offset", G_GUINT64_CONSTANT(2000000000),
		     "sync", TRUE, NULL);

	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(fakesink, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), queue, fakesink, NULL);

	if (type == KMS_MEDIA_TYPE_AUDIO) {
		dispose_audio_bw_queue(self);
		self->priv->audio_bw_queue = queue;
		dispose_audio_bw_sink(self);
		self->priv->audio_bw_sink = fakesink;
	} else if (type == KMS_MEDIA_TYPE_VIDEO) {
		dispose_video_bw_queue(self);
		self->priv->video_bw_queue = queue;
		dispose_video_bw_sink(self);
		self->priv->video_bw_sink = fakesink;
	}

	gst_element_link_many(tee, queue, fakesink, NULL);
}

guint
kms_media_handler_sink_get_bandwidth(KmsMediaHandlerSrc *self,
							KmsMediaType type) {
	if (type == KMS_MEDIA_TYPE_AUDIO) {
		return self->priv->audio_bps;
	} else if (type == KMS_MEDIA_TYPE_VIDEO) {
		return self->priv->video_bps;
	} else {
		g_warning("Requested bandwidth of an unknown type");
		return 0;
	}
}

/**
 * kms_media_handler_src_set_pad:
 *
 * @self: Self object
 * @pad: (transfer full): The new prefered pad
 * @tee: (transfer none): The tee that is connected to the pad
 * 				and where the link will be done
 * @type: The media type of the pad
 *
 * Sets the prefered connecting pad for the given type.
 */
void
kms_media_handler_src_set_pad(KmsMediaHandlerSrc *self, GstPad *pad,
					GstElement *tee, KmsMediaType type) {
	g_object_set_data(G_OBJECT(pad), TEE, tee);
	switch (type) {
	case KMS_MEDIA_TYPE_AUDIO:
		LOCK(self);
		dispose_audio_prefered_pad(self);
		self->priv->audio_prefered_pad = pad;
		create_bw_calculator(self, type, pad, tee);
		UNLOCK(self);
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		LOCK(self);
		dispose_video_prefered_pad(self);
		self->priv->video_prefered_pad = pad;
		create_bw_calculator(self, type, pad, tee);
		UNLOCK(self);
		break;
	default:
		return;
	}

	connect_pads(self);
}

/**
 * kms_media_handler_src_set_raw_pad:
 *
 * @self: Self object
 * @pad: (transfer full): The new raw pad
 * @tee: (transfer none): The tee that is connected to the pad
 * 				and where the link will be done
 * @type: The media type of the pad
 *
 * Sets the pad with raw capabilities for the given type.
 */
void
kms_media_handler_src_set_raw_pad(KmsMediaHandlerSrc *self, GstPad *pad,
					GstElement *tee, KmsMediaType type) {
	g_object_set_data(G_OBJECT(pad), TEE, tee);
	switch (type) {
		case KMS_MEDIA_TYPE_AUDIO:
			LOCK(self);
			dispose_audio_raw_pad(self);
			self->priv->audio_raw_pad = pad;
			UNLOCK(self);
			break;
		case KMS_MEDIA_TYPE_VIDEO:
			LOCK(self);
			dispose_video_raw_pad(self);
			self->priv->video_raw_pad = pad;
			UNLOCK(self);
			break;
		default:
			return;
	}

	connect_pads(self);
}

static void
constructed(GObject *object) {
	KmsMediaHandlerSrc *self = KMS_MEDIA_HANDLER_SRC(object);
	GstElement *pipe, *bin;

	bin = GST_ELEMENT(self);

	g_object_set(bin, "async-handling", TRUE, NULL);
	GST_OBJECT_FLAG_SET(bin, GST_ELEMENT_LOCKED_STATE);
	gst_element_set_state(bin, GST_STATE_PLAYING);

	pipe = kms_get_pipeline();
	gst_bin_add(GST_BIN(pipe), bin);
}

static void
finalize(GObject *object) {
	/**/

	G_OBJECT_CLASS(kms_media_handler_src_parent_class)->finalize(object);
}

static gpointer
set_to_null_state(gpointer object) {
	gst_element_set_locked_state(GST_ELEMENT(object), FALSE);
	gst_element_set_state(GST_ELEMENT(object), GST_STATE_NULL);

	g_object_unref(object);

	return NULL;
}

static void
dispose(GObject *object) {
	GstObject *parent;
	KmsMediaHandlerSrc *self = KMS_MEDIA_HANDLER_SRC(object);

	parent = gst_element_get_parent(object);

	LOCK(self);
	dispose_audio_prefered_pad(self);
	dispose_video_prefered_pad(self);
	dispose_audio_raw_pad(self);
	dispose_video_raw_pad(self);
	dispose_audio_other_pads(self);
	dispose_video_other_pads(self);
	dispose_video_bw_queue(self);
	dispose_audio_bw_queue(self);
	dispose_video_bw_sink(self);
	dispose_audio_bw_sink(self);
	g_slist_free(self->priv->encoders);
	self->priv->encoders = NULL;
	g_source_remove(self->priv->bw_source);
	UNLOCK(self);

	kms_utils_remove_src_pads(GST_ELEMENT(self));

	if (parent != NULL && GST_IS_PIPELINE(parent)) {
		/*
		 * HACK:
		 * Increase reference because it will be lost while removing
		 * from pipe
		 */
		g_object_ref(object);
		gst_bin_remove(GST_BIN(parent), GST_ELEMENT(object));
	}

	if (GST_STATE(object) != GST_STATE_NULL) {
		GThread *thread;

#if !GLIB_CHECK_VERSION(2,32,0)
		thread = g_thread_create(set_to_null_state, g_object_ref(object),
								TRUE, NULL);
#else
		thread = g_thread_new("set_to_null_state", set_to_null_state,
							g_object_ref(object));
#endif
		g_thread_unref(thread);
	} else {
		G_OBJECT_CLASS(kms_media_handler_src_parent_class)->dispose(object);
	}
}

static void
kms_media_handler_src_class_init(KmsMediaHandlerSrcClass *klass) {
	GstPadTemplate *templ;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	GST_DEBUG_CATEGORY_INIT(GST_CAT_DEFAULT, "kms-handler-src",
				GST_DEBUG_FG_CYAN, "KmsMediaHandlerSrc");

	g_type_class_add_private(klass, sizeof(KmsMediaHandlerSrcPriv));

	gobject_class->constructed = constructed;
	gobject_class->finalize = finalize;
	gobject_class->dispose = dispose;

	GST_ELEMENT_CLASS(klass)->request_new_pad = request_new_pad;

	templ = gst_static_pad_template_get(&audio_src);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);

	templ = gst_static_pad_template_get(&video_src);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);
}

static void
kms_media_handler_src_init(KmsMediaHandlerSrc *self) {
	self->priv = KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(self);

	self->priv->audio_prefered_pad = NULL;
	self->priv->video_prefered_pad = NULL;
	self->priv->audio_raw_pad = NULL;
	self->priv->video_raw_pad = NULL;
	self->priv->audio_other_pads = NULL;
	self->priv->video_other_pads = NULL;
	self->priv->video_bw_queue = NULL;
	self->priv->audio_bw_queue = NULL;
	self->priv->video_bw_sink = NULL;
	self->priv->audio_bw_sink = NULL;
	self->priv->audio_bps = 0;
	self->priv->video_bps = 0;
	self->priv->encoders = NULL;

	self->priv->bw_source = g_timeout_add(1000, bw_calculator, self);
}
