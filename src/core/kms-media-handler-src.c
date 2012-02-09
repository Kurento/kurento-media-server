#include <kms-core.h>
#include "internal/kms-utils.h"

#define KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))

#define TEE "tee"
#define MEDIA_TYPE "type"

struct _KmsMediaHandlerSrcPriv {
	GStaticMutex mutex;

	GstPad *audio_prefered_pad;
	GstPad *audio_raw_pad;
	GSList *audio_other_pads;
	GstPad *video_prefered_pad;
	GstPad *video_raw_pad;
	GSList *video_other_pads;
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

	if (sink == NULL) {
		g_warn_if_reached();
		SET_ERROR(err, KMS_MEDIA_HANDLER_SRC_ERROR,
				KMS_MEDIA_HANDLER_SRC_ERROR_LINK_ERROR,
				"Sink is NULL");
		return FALSE;
	}

	if (self == NULL) {
		g_warn_if_reached();
		SET_ERROR(err, KMS_MEDIA_HANDLER_SRC_ERROR,
				KMS_MEDIA_HANDLER_SRC_ERROR_LINK_ERROR,
				"Source is NULL");
		return FALSE;
	}

	eclass = G_ENUM_CLASS(g_type_class_peek(KMS_MEDIA_TYPE));
	evalue = g_enum_get_value(eclass, type);

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
		g_print("Pad %s not found in %s", src_name, GST_OBJECT_NAME(self));
		ret = TRUE;
		goto end;
	}
	sink_pad = gst_element_get_static_pad(GST_ELEMENT(sink), sink_name);
	if (sink_pad == NULL) {
		gst_element_release_request_pad(GST_ELEMENT(self), src_pad);
		gst_object_unref(src_pad);
		g_print("Pad %s not found int %s", sink_name, GST_OBJECT_NAME(sink));
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

	eclass = G_ENUM_CLASS(g_type_class_peek(GST_TYPE_PAD_LINK_RETURN));
	evalue = g_enum_get_value(eclass, link_ret);

	if (GST_PAD_LINK_FAILED(link_ret)) {
		SET_ERROR(err, KMS_MEDIA_HANDLER_SRC_ERROR,
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
	GstCaps *pad_caps, *intersect;
	gboolean ret;

	if (pad == NULL)
		return FALSE;

	pad_caps = GST_PAD_CAPS(pad);
	if (pad_caps != NULL) {
		gst_caps_ref(pad_caps);
	} else {
		pad_caps = gst_pad_get_caps(pad);
	}

	intersect = gst_caps_intersect(caps, pad_caps);
	ret = !gst_caps_is_empty(intersect);
	gst_caps_unref(intersect);
	gst_caps_unref(pad_caps);

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
	g_object_set(rate, "tolerance", GST_MSECOND * 250, NULL);

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

	caps = GST_PAD_CAPS(peer);
	if (caps != NULL) {
		gst_caps_ref(caps);
	} else {
		caps = gst_pad_get_caps(peer);
	}
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
	} else {
		g_object_set_data(G_OBJECT(pad), MEDIA_TYPE,
					GINT_TO_POINTER(KMS_MEDIA_TYPE_UNKNOWN));
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
		UNLOCK(self);
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		LOCK(self);
		dispose_video_prefered_pad(self);
		self->priv->video_prefered_pad = pad;
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
		gst_element_set_locked_state(GST_ELEMENT(object), FALSE);
		gst_element_set_state(GST_ELEMENT(object), GST_STATE_NULL);
	}

	G_OBJECT_CLASS(kms_media_handler_src_parent_class)->dispose(object);
}

static void
kms_media_handler_src_class_init(KmsMediaHandlerSrcClass *klass) {
	GstPadTemplate *templ;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

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
}
