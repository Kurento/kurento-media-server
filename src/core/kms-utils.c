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

#include <gst/gst.h>
#include "kms-core.h"
#include "internal/kms-utils.h"

#define ATTR_DYNAMIC "dynamic"
#define ATTR_REMOVE "remove"

#define GST_CAT_DEFAULT kms_utils
GST_DEBUG_CATEGORY_STATIC(GST_CAT_DEFAULT);

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
		KMS_DEBUG_PIPE("error");
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
		GThread *thread;

		g_type_init();
		gst_init(argc, argv);

		pipe = gst_pipeline_new(NULL);
		gst_element_set_state(pipe, GST_STATE_PLAYING);

		bus = gst_element_get_bus(pipe);
		gst_bus_add_watch(bus, gst_bus_async_signal_func, NULL);
		g_object_connect(bus, "signal::message", bus_msg, NULL, NULL);

#if !GLIB_CHECK_VERSION(2,32,0)
		thread = g_thread_create(gstreamer_thread, NULL, TRUE, NULL);
#else
		thread = g_thread_new("main_thread", gstreamer_thread, NULL);
#endif
		g_thread_unref(thread);

		init = TRUE;

		GST_DEBUG_CATEGORY_INIT(GST_CAT_DEFAULT, "kms-utils",
					GST_DEBUG_FG_CYAN, "kms utils file");
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
	if (!gst_element_link(orig, dest)) {
		g_print("Error linking %s and %s\n", GST_ELEMENT_NAME(orig),
							GST_ELEMENT_NAME(dest));
	}
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

static gboolean
remove_from_bin(GstElement *elem) {
	GstObject *parent;

	parent = gst_element_get_parent(elem);
	if (parent != NULL) {
		gst_bin_remove(GST_BIN(parent), elem);
	}

	gst_element_set_state(elem, GST_STATE_NULL);
	g_object_unref(elem);

	return FALSE;
}

static void
unlinked(GstPad *pad, gpointer orig) {
	GstElement *dest;
	GstPad *peer, *sink;
	gboolean dynamic, remove;

	remove = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pad), ATTR_REMOVE));
	dest = gst_pad_get_parent_element(pad);
	sink = gst_element_get_static_pad(dest, "sink");
	GST_OBJECT_LOCK(dest);
	if (gst_pad_is_linked(sink) && !check_sources_linked(dest)) {
		peer = gst_pad_get_peer(sink);
		if (peer != NULL) {
			gst_pad_unlink(peer, sink);
			g_object_unref(peer);
			if (remove) {
				g_timeout_add(100, (GSourceFunc) remove_from_bin,
							g_object_ref(dest));
			}
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
connect_pad_callbacks(GstElement *orig, GstPad *pad, gboolean dynamic,
							gboolean remove) {
	g_object_set_data(G_OBJECT(pad), ATTR_DYNAMIC, GINT_TO_POINTER(dynamic));
	g_object_set_data(G_OBJECT(pad), ATTR_REMOVE, GINT_TO_POINTER(remove));

	g_object_connect(pad, "signal::linked", linked, orig, NULL);
	g_object_connect(pad, "signal::unlinked", unlinked, orig, NULL);
}

void
kms_dynamic_connection(GstElement *orig, GstElement *dest,
						const gchar *pad_name) {
	kms_dynamic_connection_full(orig, dest, pad_name, FALSE);
}

void
kms_dynamic_connection_full(GstElement *orig, GstElement *dest,
							const gchar *pad_name,
							gboolean remove) {
	GstPad *pad;

	pad = gst_element_get_static_pad(dest, pad_name);
	connect_pad_callbacks(orig, pad, FALSE, remove);
	g_object_unref(pad);
}

static void
tee_pad_added(GstElement *tee, GstPad *pad, GstElement *orig) {
	connect_pad_callbacks(orig, pad, TRUE, FALSE);
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

static void
set_fixed(GstCaps *caps, const gchar *field) {
	guint i;

	for (i = 0; i < gst_caps_get_size(caps); i++) {
		GstStructure *st;

		st = gst_caps_get_structure(caps, i);

		if (gst_structure_has_field_typed(st, field, G_TYPE_INT)) {
			// Already fixed
		} else if (gst_structure_has_field(st, field)) {
			const GValue *val;
			gint max;

			val = gst_structure_get_value(st, field);
			if (val == NULL || !GST_VALUE_HOLDS_INT_RANGE(val)) {
				gst_structure_remove_field(st, field);
			}

			max = gst_value_get_int_range_max(val);
			gst_structure_set(st, field, G_TYPE_INT, max, NULL);
		}
	}
}

static void
set_range(GstCaps *caps, const gchar *field) {
	guint i;

	for (i = 0; i < gst_caps_get_size(caps); i++) {
		GstStructure *st;
		gint max;

		st = gst_caps_get_structure(caps, i);

		if (gst_structure_get_int(st, field, &max)) {
			gst_structure_set(st, field, GST_TYPE_INT_RANGE,
								0, max, NULL);

		} else if (gst_structure_has_field(st, field)) {
			const GValue *val;

			val = gst_structure_get_value(st, field);
			if (val == NULL || !GST_VALUE_HOLDS_INT_RANGE(val)) {
				gst_structure_remove_field(st, field);
			}

			// Already int range
		}
	}
}

static void
set_as_src_caps(GstCaps *caps) {
	set_fixed(caps, "bandwidth");
	set_fixed(caps, "width");
	set_fixed(caps, "height");
}

static void
set_as_sink_caps(GstCaps *caps) {
	set_range(caps, "bandwidth");
	set_range(caps, "width");
	set_range(caps, "height");
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

	// Source has a fixed bandwidth while sink has it open from 0 to max
	set_as_src_caps(copy_src_caps);
	set_as_sink_caps(copy_sink_caps);

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
	if (parent != NULL) {
		gst_bin_remove(GST_BIN(parent), elem);
		g_object_unref(parent);
	}

	gst_element_set_state(elem, GST_STATE_NULL);
	g_object_unref(elem);
	return;
}

void
kms_utils_connect_target_with_queue(GstElement *elem, GstGhostPad *gp) {
	GstElement *queue = NULL;
	GstObject *parent;
	GstPad *src;

	parent = gst_element_get_parent(elem);
	if (parent == NULL)
		return;

	queue = kms_utils_create_queue(NULL);
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
		if (!gst_element_link(elem, queue)) {
			g_warn_if_reached();
			g_print("Error Linking %s and %s\n",
						GST_ELEMENT_NAME(elem),
						GST_ELEMENT_NAME(queue));
			KMS_DEBUG_PIPE("error_linking");
		}
	} else {
		gst_bin_remove(GST_BIN(parent), queue);
	}

	g_object_unref(src);

end:
	g_object_unref(parent);
}

void
kms_utils_remove_when_unlinked(GstPad *pad) {
	if (!GST_IS_PAD(pad))
		return;

	g_object_connect(pad, "signal::unlinked", unlinked_delete, NULL, NULL);
}

void
kms_utils_remove_when_unlinked_pad_name(GstElement *elem, const gchar* pad_name) {
	GstPad *pad;

	pad = gst_element_get_static_pad(elem, pad_name);
	g_return_if_fail(pad != NULL);

	kms_utils_remove_when_unlinked(pad);

	g_object_unref(pad);
}

gint
kms_utils_get_bandwidth_from_caps(GstCaps *caps) {
	gint i, retval = -1, aux;

	if (caps == NULL)
		return -1;

	for (i = 0; i < gst_caps_get_size(caps); i++) {
		GstStructure *st;
		aux = -1;

		st = gst_caps_get_structure(caps, i);

		if (gst_structure_get_int(st, "bandwidth", &aux)) {
			if (retval == -1 || aux < retval) {
				retval = aux;
			}
		} else if (gst_structure_has_field(st, "bandwidth")) {
			const GValue *val;

			val = gst_structure_get_value(st, "bandwidth");
			if (val == NULL || !GST_VALUE_HOLDS_INT_RANGE(val))
				continue;

			aux = gst_value_get_int_range_max(val);
		}

		if (retval == -1 || (aux > 0 && aux < retval)) {
			retval = aux;
		}
	}

	return retval;
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
		g_object_set(G_OBJECT(elem), "config-interval", 2,
						"send-config", TRUE, NULL);
	} else if (g_strcmp0(name, "xvidenc") == 0) {
		g_object_set(G_OBJECT(elem), "max-bquant", 31,
						"min-bquant", 31,
						"bquant-ratio", 0,
						"max-bframes", 0,
						"max-iquant", 20,
						"min-iquant", 0,
						"max-pquant", 20,
						"min-pquant", 0,
						"motion", 6,
						"trellis", TRUE,
						"lumimasking", TRUE,
						"quant-type", 1,
						"profile", 148,
						"max-key-interval", 5,
						"bitrate", 300000, NULL);
	} else if (g_strcmp0(name, "rtph264pay") == 0) {
		g_object_set(G_OBJECT(elem), "config-interval", 5, NULL);
	} else if (g_strcmp0(name, "x264enc") == 0) {
		g_object_set(G_OBJECT(elem), "vbv-buf-capacity", 0,
						"profile", 0,
						"trellis", FALSE,
						"speed-preset", 2,
						"sync-lookahead", 0,
						"rc-lookahead", 0,
						"b-adapt", FALSE,
						"pb-factor", 2.0,
						"mb-tree", FALSE,
						"pass", 17 /* pass1 */,
						"bitrate", 300, NULL);
	} else if (g_strcmp0(name, "ffenc_mpeg4") == 0) {
		g_object_set(G_OBJECT(elem), "bitrate", 300000,
						"qcompress", 0.0,
						"qblur", 0.0,
						"gop-size", 5,
						"trellis", 0, NULL);
	}
}

void
kms_utils_configure_bw(GstElement *elem, guint neg_bw, guint bw) {
	GstElementFactory *factory;
	gchar *name;
	gint final_bw, current_bw, diff;


	factory = gst_element_get_factory(elem);
	if (factory == NULL)
		return;

	name = GST_OBJECT_NAME(factory);
	if (name == NULL)
		return;

	if (bw == 0)
		return;

	if (neg_bw == 0) {
		final_bw = bw;
	} else {
		final_bw = bw < neg_bw ? (bw + neg_bw) / 2 : neg_bw;
	}

	if (g_strcmp0(name, "xvidenc") == 0 ||
				g_strcmp0(name, "ffenc_mpeg4") == 0 ||
				g_strcmp0(name, "ffenc_h263") == 0 ||
				g_strcmp0(name, "ffenc_h263p") == 0 ||
				g_strcmp0(name, "ffenc_flv") == 0) {
		g_object_get(G_OBJECT(elem), "bitrate", &current_bw, NULL);

		diff = final_bw > current_bw ? final_bw - current_bw :
							current_bw - final_bw;

		if (diff > 70000) {
			GST_DEBUG("Setting bw of %P to: %d, current: %d", elem,
							final_bw, current_bw);
			gst_element_set_state(elem, GST_STATE_READY);
			g_object_set(G_OBJECT(elem), "bitrate", final_bw, NULL);
			gst_element_set_state(elem, GST_STATE_PLAYING);
		}
	} else if (g_strcmp0(name, "x264enc") == 0) {
		gint x264bw;

		g_object_get(G_OBJECT(elem), "bitrate", &current_bw, NULL);
		x264bw = final_bw / 1000;

		diff = x264bw > current_bw ? x264bw - current_bw :
		current_bw - x264bw;

		if (diff > 70) {
			GST_DEBUG("Setting bw of %P to: %d, current: %d", elem,
							x264bw, current_bw);
			gst_element_set_state(elem, GST_STATE_READY);
			g_object_set(G_OBJECT(elem), "bitrate", x264bw, NULL);
			gst_element_set_state(elem, GST_STATE_PLAYING);
		}
	} else {
		GST_WARNING("Unknown factory: %s", name);
	}
}

static void
transfer_structure(const GstStructure *st, GstCaps *to) {
	gint width, height, fr_num, fr_denom, rate, clockrate, bandwidth;
	gint i;

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

	if (!gst_structure_get_int(st, "bandwidth", &bandwidth))
		bandwidth = 0;

	/* Set this attributes in all the destination structs */
	for (i = 0; i < gst_caps_get_size(to); i++) {
		GstStructure *dest;
		GstStructure *dest_orig;

		dest = gst_caps_get_structure(to, i);
		dest_orig = gst_structure_copy(dest);
		if (width != 0)
			gst_structure_set(dest, "width", G_TYPE_INT, width,
									NULL);

		if (height != 0)
			gst_structure_set(dest, "height", G_TYPE_INT, height,
									NULL);

		if (fr_num != 0 || fr_denom != 0)
			gst_structure_set(dest, "framerare", GST_TYPE_FRACTION,
							fr_num, fr_denom, NULL);

		if (rate != 0) {
			gst_structure_set(dest, "rate", G_TYPE_INT, rate, NULL);
			if (clockrate == 0)
				gst_structure_set(dest, "clock-rate",
							G_TYPE_INT, rate, NULL);
		}

		if (clockrate != 0) {
			gst_structure_set(dest, "clock-rate", G_TYPE_INT,
							clockrate, NULL);
			if (rate == 0)
				gst_structure_set(dest, "rate", G_TYPE_INT,
							clockrate, NULL);
		}

		if (bandwidth != 0)
			gst_structure_set(dest, "bandwidth", G_TYPE_INT,
							bandwidth, NULL);

		if (!gst_structure_can_intersect(dest, dest_orig)) {
			gst_caps_remove_structure(to, i);
			/* Restart processing */
			i = 0;
		}
		gst_structure_free(dest_orig);
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

GstElement*
kms_utils_create_queue(const gchar *name) {
	GstElement *queue;

	queue = gst_element_factory_make("queue", name);
	if (queue != NULL) {
		g_object_set(queue, "max-size-buffers", 4, "leaky", 2,
							"silent", TRUE, NULL);
	}

	return queue;
}

GstElement*
kms_utils_create_fakesink(const gchar *name) {
	GstElement *fakesink;

	fakesink = gst_element_factory_make("fakesink", name);
	if (fakesink != NULL) {
		g_object_set(fakesink, "enable-last-buffer", FALSE,
							"silent", TRUE, NULL);
	}

	return fakesink;
}

gchar*
kms_utils_generate_pad_name(gchar *pattern) {
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
remove_pad(GstPad *pad, GstElement *elem) {
	gst_element_release_request_pad(elem, pad);
	gst_object_unref(pad);
}

void
kms_utils_remove_src_pads(GstElement *self) {
	GstIterator *it = gst_element_iterate_src_pads(self);

	gst_iterator_foreach(it, (GFunc) remove_pad, self);
	gst_iterator_free(it);
}

void
kms_utils_remove_sink_pads(GstElement *self) {
	GstIterator *it = gst_element_iterate_sink_pads(self);

	gst_iterator_foreach(it, (GFunc) remove_pad, self);
	gst_iterator_free(it);
}

static void
pad_unlinked(GstPad *pad, gpointer not_used) {
	GstElement *elem;

	elem = gst_pad_get_parent_element(pad);

	if (GST_IS_ELEMENT(elem))
		gst_element_release_request_pad(elem, pad);
}

static void
pad_added(GstElement *tee, GstPad *pad, gpointer not_used) {
	g_object_connect(pad, "signal::unlinked", pad_unlinked, NULL, NULL);
}

void
kms_utils_release_unlinked_pads(GstElement *elem) {
	g_object_connect(elem, "signal::pad_added", pad_added, NULL, NULL);
}

gboolean
kms_g_ptr_array_contains(GPtrArray *array, gpointer value) {
	gint i;

	if (array == NULL)
		return FALSE;

	for (i = 0; i < array->len; i++) {
		gpointer current = g_ptr_array_index(array, i);

		if (current == value)
			return TRUE;
	}
	return FALSE;
}
