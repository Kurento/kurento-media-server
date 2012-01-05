#include <kms-core.h>
#include "internal/kms-utils.h"

#define KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))

G_LOCK_DEFINE_STATIC(class_lock);

struct _KmsMediaHandlerSrcPriv {
	GStaticMutex mutex;
};

enum {
	PROP_0,

};

G_DEFINE_TYPE(KmsMediaHandlerSrc, kms_media_handler_src, GST_TYPE_BIN)

static gchar*
get_name(KmsMediaHandlerSrc *self) {
	static glong count = 0;
	gchar *name;

	G_LOCK(class_lock);
	name = g_strdup_printf("%s-%ld", G_OBJECT_TYPE_NAME(self), count++);
	G_UNLOCK(class_lock);

	return name;
}

static void
constructed(GObject *object) {
	KmsMediaHandlerSrc *self = KMS_MEDIA_HANDLER_SRC(object);
	GstElement *pipe, *bin;
	gchar *name;

	name = get_name(self);
	gst_element_set_name(self, name);
	g_free(name);

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

	G_OBJECT_CLASS(kms_media_handler_src_parent_class)->dispose(object);
}

static void
kms_media_handler_src_class_init(KmsMediaHandlerSrcClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsMediaHandlerSrcPriv));

	gobject_class->constructed = constructed;
	gobject_class->finalize = finalize;
	gobject_class->dispose = dispose;
}

static void
kms_media_handler_src_init(KmsMediaHandlerSrc *self) {
	self->priv = KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(self);

}
