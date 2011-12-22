#include <gst/gst.h>
#include "kms-core.h"

static GstElement *pipe = NULL;
static G_LOCK_DEFINE(mutex);
static gboolean init = FALSE;

static gboolean
bus_watch(GstBus *bus, GstMessage *message, gpointer data) {
	KMS_LOG_DEBUG("TODO: implement bus watcher\n");
	return TRUE;
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
		gst_bus_add_watch(bus, bus_watch, NULL);
		init = TRUE;
	}
	G_UNLOCK(mutex);
}

GstElement*
kms_get_pipeline() {
	return pipe;
}
