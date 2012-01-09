#include <kms-core.h>
#include "internal/kms-utils.h"

#define KMS_MEDIA_HANDLER_SINK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSinkPriv))

#define LOCK(obj) (g_mutex_lock(&(KMS_MEDIA_HANDLER_SINK(obj)->priv->mutex)))
#define UNLOCK(obj) (g_mutex_unlock(&(KMS_MEDIA_HANDLER_SINK(obj)->priv->mutex)))

struct _KmsMediaHandlerSinkPriv {
	GMutex *mutex;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsMediaHandlerSink, kms_media_handler_sink, GST_TYPE_BIN)

gboolean
kms_media_handler_sink_disconnect(KmsMediaHandlerSink *self,
						KmsMediaHandlerSrc *src,
						GError **err) {
	g_print("Disconnect sink\n");
	return TRUE;
}

static void
constructed(GObject *object) {
	GstElement *pipe, *bin;

	bin = GST_ELEMENT(object);

	g_object_set(bin, "async-handling", TRUE, NULL);
	GST_OBJECT_FLAG_SET(bin, GST_ELEMENT_LOCKED_STATE);
	gst_element_set_state(bin, GST_STATE_PLAYING);

	pipe = kms_get_pipeline();
	gst_bin_add(GST_BIN(pipe), bin);
}

static void
finalize(GObject *object) {
	KmsMediaHandlerSink *self = KMS_MEDIA_HANDLER_SINK(object);

	g_mutex_free(self->priv->mutex);

	G_OBJECT_CLASS(kms_media_handler_sink_parent_class)->finalize(object);
}

static void
dispose(GObject *object) {
	GstObject *parent;

	parent = gst_element_get_parent(object);

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

	G_OBJECT_CLASS(kms_media_handler_sink_parent_class)->dispose(object);
}

static void
kms_media_handler_sink_class_init(KmsMediaHandlerSinkClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsMediaHandlerSinkPriv));

	gobject_class->constructed = constructed;
	gobject_class->finalize = finalize;
	gobject_class->dispose = dispose;
}

static void
kms_media_handler_sink_init(KmsMediaHandlerSink *self) {
	self->priv = KMS_MEDIA_HANDLER_SINK_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
