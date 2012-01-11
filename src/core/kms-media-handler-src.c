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
		SET_ERROR(err, KMS_MEDIA_HANDLER_SRC_ERROR,
				KMS_MEDIA_HANDLER_SRC_ERROR_PAD_NOT_FOUND,
				"Pad was not found for source element");
		ret = FALSE;
		goto end;
	}
	sink_pad = gst_element_get_static_pad(GST_ELEMENT(sink), sink_name);
	if (sink_pad == NULL) {
		SET_ERROR(err, KMS_MEDIA_HANDLER_SRC_ERROR,
				KMS_MEDIA_HANDLER_SRC_ERROR_PAD_NOT_FOUND,
				"Pad was not found for sink element");
		ret = FALSE;
		gst_element_release_request_pad(GST_ELEMENT(self), src_pad);
		goto end;
	}

	if (gst_pad_is_linked(sink_pad)) {
		GstPad *tmp_src;
		tmp_src = gst_pad_get_peer(sink_pad);
		gst_pad_unlink(tmp_src, sink_pad);
		g_object_unref(tmp_src);
	}

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

static gchar*
generate_pad_name(gchar *pattern) {
	static int n = 0;
	G_LOCK_DEFINE_STATIC(pad_n);
	GString *name;
	gchar *name_str;

	if (g_str_has_suffix(pattern, "%d")) {
		name = g_string_new(pattern);
		name->str[name->len - 2] = '\0';
		name->len = name->len -2;

		G_LOCK(pad_n);
		g_string_append_printf(name, "%d", n++);
		G_UNLOCK(pad_n);

		name_str = name->str;
		g_string_free(name, FALSE);
	} else {
		G_LOCK(pad_n);
		name_str = g_strdup_printf("pad_name%d", n++);
		G_UNLOCK(pad_n);
	}

	return name_str;
}

static void
pad_unlinked(GstPad  *pad, GstPad  *peer, GstElement *elem) {
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);
	gst_element_release_request_pad(elem, pad);
	KMS_DEBUG_PIPE("unlinked");
}

static gboolean
check_pad_compatible(GstPad *pad, GstCaps *caps) {
	GstCaps *pad_caps;
	gboolean ret;

	if (pad == NULL)
		return FALSE;

	pad_caps = GST_PAD_CAPS(pad);
	if (pad_caps != NULL) {
		gst_caps_ref(pad_caps);
	} else {
		pad_caps = gst_pad_get_caps(pad);
	}

	ret = gst_caps_can_intersect(caps, pad_caps);
	gst_caps_unref(pad_caps);

	return ret;
}

static GstPad*
get_target_pad(GstPad *peer, GstPad *prefered, GstPad *raw, GSList **other) {
	GstCaps *caps;
	GstPad *target = NULL;

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
	/* TODO: Check raw pad */
	/* TODO: Check other pads */

end:
	gst_caps_unref(caps);
	return target;
}

static void
set_target_pad(GstGhostPad *gp, GstPad *target) {
	GstElement *tee;
	GstPad *real_target;

	tee = g_object_get_data(G_OBJECT(target), TEE);
	if (tee == NULL)
		return;

	real_target = gst_element_get_request_pad(tee, "src%d");
	if (real_target != NULL) {
		gst_ghost_pad_set_target(gp, real_target);
		g_object_unref(real_target);
	}
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
		target_pad = get_target_pad(peer, self->priv->audio_prefered_pad,
					self->priv->audio_prefered_pad,
					&(self->priv->audio_other_pads));
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		target_pad = get_target_pad(peer, self->priv->video_prefered_pad,
					self->priv->video_prefered_pad,
					&(self->priv->video_other_pads));
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
		new_name = generate_pad_name(templ->name_template);

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
	GstGhostPad *pad;
	GstPad *peer;
	gboolean done = FALSE;

	it = gst_element_iterate_src_pads(GST_ELEMENT(self));

	while (!done) {
		switch (gst_iterator_next(it, (gpointer *)&pad)) {
		case GST_ITERATOR_OK:
			if (gst_ghost_pad_get_target(pad) == NULL &&
					gst_pad_is_linked(GST_PAD(pad))) {
				/* TODO: Possible race condigion, pad can be
				* 	unlinked during this process */
				peer = gst_pad_get_peer(GST_PAD(pad));
				link_pad(GST_PAD(pad), peer);
				g_object_unref(peer);
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
