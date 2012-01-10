#include <kms-core.h>
#include "internal/kms-utils.h"

#define KMS_MEDIA_HANDLER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SRC, KmsMediaHandlerSrcPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_MEDIA_HANDLER_SRC(obj)->priv->mutex)))

struct _KmsMediaHandlerSrcPriv {
	GStaticMutex mutex;
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

gboolean
kms_media_handler_src_connect(KmsMediaHandlerSrc *self,
						KmsMediaHandlerSink *src,
						GError **err) {
	g_print("Connect src\n");
	return TRUE;
}

static GstPad*
request_new_pad(GstElement *elem, GstPadTemplate *templ, const gchar *name) {
	GstPad *pad;

	pad = gst_ghost_pad_new_no_target_from_template(name, templ);
	/* TODO: Connect pad callbacks */
	return pad;
}

static void
release_pad(GstElement *elem, GstPad *pad) {
	g_print("Release pad\n");
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
	GstPadTemplate *templ;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsMediaHandlerSrcPriv));

	gobject_class->constructed = constructed;
	gobject_class->finalize = finalize;
	gobject_class->dispose = dispose;

	GST_ELEMENT_CLASS(klass)->request_new_pad = request_new_pad;
	GST_ELEMENT_CLASS(klass)->release_pad = release_pad;

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
}
