#include <gst/gst.h>
#include "kms-core.h"

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

void
kms_dynamic_connection(GstElement *orig, GstElement *dest, gchar *pad_name) {
	/* TODO: implement this function*/
}
