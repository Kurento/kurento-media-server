#include <gst/gst.h>
#include "kms-core.h"
#include "internal/kms-utils.h"

#define ATTR_DYNAMIC "dynamic"

static GstElement *pipe = NULL;
G_LOCK_DEFINE_STATIC(mutex);
static gboolean init = FALSE;

static gpointer
gstreamer_thread(gpointer data) {
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	return NULL;
}

static void
bus_msg(GstBus *bus, GstMessage *msg, gpointer not_used) {
	switch (msg->type) {
	case GST_MESSAGE_ERROR: {
		GError *err = NULL;
		gchar *dbg_info = NULL;

		gst_message_parse_error (msg, &err, &dbg_info);
		g_warning("ERROR from element %s: %s\n",
						GST_OBJECT_NAME (msg->src),
						err->message);
		g_printerr("Debugging info: %s\n",
						(dbg_info) ? dbg_info : "none");
		g_error_free(err);
		g_free(dbg_info);
		break;
	}
	case GST_MESSAGE_EOS:
		g_warning("EOS message should not be received, "
						"pipeline can be stopped!");
		break;
	default:
		/* No action */
		break;
	}


}

void
kms_init(gint *argc, gchar **argv[]) {
	G_LOCK(mutex);
	if (!init) {
		GstBus *bus;

		g_type_init();
		gst_init(argc, argv);
		pipe = gst_pipeline_new(NULL);
		gst_element_set_state(pipe, GST_STATE_PLAYING);

		bus = gst_element_get_bus(pipe);
		gst_bus_add_watch(bus, gst_bus_async_signal_func, NULL);
		g_object_connect(bus, "signal::message", bus_msg, NULL, NULL);

		g_thread_create(gstreamer_thread, NULL, TRUE, NULL);

		init = TRUE;
	}
	G_UNLOCK(mutex);
}

GstElement*
kms_get_pipeline() {
	return pipe;
}

static void
linked(GstPad *pad, GstPad *peer, gpointer orig) {
	GstElement *dest;

	dest = gst_pad_get_parent_element(pad);
	gst_element_link(orig, dest);
	g_object_unref(dest);
}

static gboolean
check_sources_linked(GstElement *elem) {
	GList *l;
	GstPad *pad;

	l = elem->srcpads;
	while (l != NULL) {
		pad = l->data;
		if (pad != NULL && gst_pad_is_linked(pad))
			return TRUE;
		l = l->next;
	}

	return FALSE;
}

static void
unlinked(GstPad *pad, gpointer orig) {
	GstElement *dest;
	GstPad *peer, *sink;
	gboolean dynamic;

	dest = gst_pad_get_parent_element(pad);
	sink = gst_element_get_static_pad(dest, "sink");
	GST_OBJECT_LOCK(dest);
	if (gst_pad_is_linked(sink) && !check_sources_linked(dest)) {
		peer = gst_pad_get_peer(sink);
		if (peer != NULL) {
			gst_pad_unlink(peer, sink);
			g_object_unref(peer);
		}
	}

	GST_OBJECT_UNLOCK(dest);
	dynamic = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pad), ATTR_DYNAMIC));
	if (dynamic)
		gst_element_release_request_pad(dest, pad);

	g_object_unref(dest);
	g_object_unref(sink);
}

static void
connect_pad_callbacks(GstElement *orig, GstPad *pad, gboolean dynamic) {
	g_object_set_data(G_OBJECT(pad), ATTR_DYNAMIC, GINT_TO_POINTER(dynamic));

	g_object_connect(pad, "signal::linked", linked, orig, NULL);
	g_object_connect(pad, "signal::unlinked", unlinked, orig, NULL);
}

void
kms_dynamic_connection(GstElement *orig, GstElement *dest,
						const gchar *pad_name) {
	GstPad *pad;

	pad = gst_element_get_static_pad(dest, pad_name);
	connect_pad_callbacks(orig, pad, FALSE);
	g_object_unref(pad);
}

static void
tee_pad_added(GstElement *tee, GstPad *pad, GstElement *orig) {
	connect_pad_callbacks(orig, pad, TRUE);
}

void
kms_dynamic_connection_tee(GstElement *orig, GstElement *tee) {
	g_object_connect(tee, "signal::pad-added", tee_pad_added, orig, NULL);
}

static gboolean
check_template_names(GstPadTemplate *tmp1, GstPadTemplate *tmp2) {
	gboolean ret = FALSE;

	switch (tmp1->direction) {
	case GST_PAD_SINK:
		ret = ret || g_strcmp0(tmp1->name_template, "sink") == 0;
		break;
	case GST_PAD_SRC:
		ret = ret || g_strcmp0(tmp1->name_template, "src") == 0;
		break;
	default:
		return FALSE;
	}

	switch (tmp2->direction) {
	case GST_PAD_SINK:
		ret = ret || g_strcmp0(tmp2->name_template, "sink") == 0;
		break;
	case GST_PAD_SRC:
		ret = ret || g_strcmp0(tmp2->name_template, "src") == 0;
		break;
	default:
		return FALSE;
	}

	return ret;
}

GstElement*
kms_utils_get_element_for_caps(GstElementFactoryListType type, GstRank rank,
				const GstCaps *caps, GstPadDirection direction,
				gboolean subsetonly, const gchar *name) {
	GList *list, *filter, *l;
	GstElement *elem = NULL;

	list = gst_element_factory_list_get_elements(type, rank);
	filter = gst_element_factory_list_filter(list, caps, direction,
								subsetonly);

	l = filter;
	while (l != NULL) {
		if (l->data != NULL) {
			const GList *pads;
			pads = gst_element_factory_get_static_pad_templates(
								l->data);
			if (g_list_length((GList *)pads) == 2) {
				GstPadTemplate *tmp1, *tmp2;
				gboolean finish = FALSE;

				tmp1 = gst_static_pad_template_get(pads->data);
				tmp2 = gst_static_pad_template_get(
							pads->next->data);

				finish = check_template_names(tmp1, tmp2);

				g_object_unref(tmp1);
				g_object_unref(tmp2);

				if (finish)
					break;
			}
		}
		l = l->next;
	}

	if (l == NULL || l->data == NULL)
		goto end;

	elem = gst_element_factory_create(l->data, name);

end:
	gst_plugin_feature_list_free(list);
	gst_plugin_feature_list_free(filter);

	return elem;
}

GstElement*
kms_generate_bin_with_caps(GstElement *elem, GstCaps *sink_caps,
							GstCaps *src_caps) {
	GstElement *bin;
	GstPad *gsink, *gsrc, *sink, *src;
	GstPadTemplate *sink_temp, *src_temp;
	GstCaps *copy_sink_caps, *copy_src_caps;

	bin = gst_bin_new(GST_ELEMENT_NAME(elem));
	gst_bin_add(GST_BIN(bin), elem);

	src = gst_element_get_static_pad(elem, "src");
	sink = gst_element_get_static_pad(elem, "sink");

	if (sink_caps != NULL)
		copy_sink_caps = gst_caps_copy(sink_caps);
	else
		copy_sink_caps = gst_pad_get_caps(sink);

	if (src_caps != NULL)
		copy_src_caps = gst_caps_copy(src_caps);
	else
		copy_src_caps = gst_pad_get_caps(src);

	sink_temp = gst_pad_template_new("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
								copy_sink_caps);
	src_temp = gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS,
								copy_src_caps);

	gsink = gst_ghost_pad_new_no_target_from_template("sink", sink_temp);
	gsrc =  gst_ghost_pad_new_no_target_from_template("src", src_temp);

	gst_element_add_pad(bin, gsink);
	gst_element_add_pad(bin, gsrc);

	gst_ghost_pad_set_target(GST_GHOST_PAD(gsrc), src);
	gst_ghost_pad_set_target(GST_GHOST_PAD(gsink), sink);

	g_object_unref(src);
	g_object_unref(sink);
	g_object_unref(sink_temp);
	g_object_unref(src_temp);
	return bin;
}

static void
unlinked_delete(GstPad *pad, GstPad *peer, gpointer not_used) {
	GstElement *elem;
	GstObject *parent;

	elem = gst_pad_get_parent_element(pad);
	if (elem == NULL)
		return;

	parent = gst_element_get_parent(elem);
	if (parent == NULL) {
		g_object_unref(elem);
		return;
	}

	gst_bin_remove(GST_BIN(parent), elem);
}

void
kms_utils_connect_target_with_queue(GstElement *elem, GstGhostPad *gp) {
	GstElement *queue = NULL;
	GstObject *parent;
	GstPad *src;

	parent = gst_element_get_parent(elem);
	if (parent == NULL)
		return;

	queue = gst_element_factory_make("queue2", NULL);
	if (queue == NULL)
		goto end;

	src = gst_element_get_static_pad(queue, "src");
	if (src == NULL) {
		g_object_unref(queue);
		goto end;
	}

	g_object_connect(src, "signal::unlinked", unlinked_delete, NULL, NULL);

	gst_bin_add(GST_BIN(parent), queue);

	if (gst_ghost_pad_set_target(gp, src)) {
		gst_element_set_state(queue, GST_STATE_PLAYING);
		gst_element_link(elem, queue);
	} else {
		gst_bin_remove(GST_BIN(parent), queue);
	}

	g_object_unref(src);

end:
	g_object_unref(parent);
}

void
kms_utils_configure_element(GstElement *elem) {
	/* TODO: This function should be pluggable or configurable */
	GstElementFactory *factory;
	gchar *name;

	factory = gst_element_get_factory(elem);
	if (factory == NULL)
		return;

	name = GST_OBJECT_NAME(factory);
	if (name == NULL)
		return;

	if (g_strcmp0(name, "rtpmp4vpay") == 0) {
		g_object_set(G_OBJECT(elem), "config-interval", 2, NULL);
	}
}

static void
transfer_structure(const GstStructure *st, GstCaps *to) {
	gint width, height, fr_num, fr_denom, rate, clockrate;
	gint i, len;

	if (!gst_structure_get_int(st, "width", &width))
		width = 0;

	if (!gst_structure_get_int(st, "height", &height))
		height = 0;

	if (!gst_structure_get_fraction(st, "framerate", &fr_num, &fr_denom)) {
		fr_num = 0;
		fr_denom = 0;
	}

	if (!gst_structure_get_int(st, "rate", &rate))
		rate = 0;

	if (!gst_structure_get_int(st, "clock-rate", &clockrate))
		clockrate = 0;

	/* Set this attributes in all the destination structs */
	len = gst_caps_get_size(to);
	for (i = 0; i < len; i++) {
		GstStructure *dest;

		dest = gst_caps_get_structure(to, i);
		if (width != 0)
			gst_structure_set(dest, "width", G_TYPE_INT, width,
									NULL);

		if (height != 0)
			gst_structure_set(dest, "height", G_TYPE_INT, height,
									NULL);

		if (fr_num != 0 || fr_denom != 0)
			gst_structure_set(dest, "framerare", GST_TYPE_FRACTION,
					  fr_num, fr_denom, NULL);

		if (rate != 0)
			gst_structure_set(dest, "rate", G_TYPE_INT, rate, NULL);

		if (clockrate != 0)
			gst_structure_set(dest, "clock-rate", G_TYPE_INT,
							clockrate, NULL);
	}
}

void
kms_utils_transfer_caps(const GstCaps *from, GstCaps *to) {
	gint i, len;

	len = gst_caps_get_size(from);
	for (i = 0; i < len; i++) {
		GstStructure *st;

		st = gst_caps_get_structure(from, i);
		transfer_structure(st, to);
	}
}
