#include <gst/gst.h>
#include "kms-core.h"

static GstElement *pipe = NULL;
static G_LOCK_DEFINE(mutex);
static gboolean init = FALSE;

static gpointer
gstreamer_thread(gpointer data) {
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	return NULL;
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

		g_thread_create(gstreamer_thread, NULL, TRUE, NULL);

		init = TRUE;
	}
	G_UNLOCK(mutex);
}

GstElement*
kms_get_pipeline() {
	return pipe;
}
