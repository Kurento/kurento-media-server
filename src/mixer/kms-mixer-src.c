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

#include <kms-core.h>
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"
#include <mixer/kms-mixer-src.h>

#define KMS_MIXER_SRC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MIXER_SRC, KmsMixerSrcPriv))

#define LOCK(obj) (g_mutex_lock(&(KMS_MIXER_SRC(obj)->priv->mutex)))
#define UNLOCK(obj) (g_mutex_unlock(&(KMS_MIXER_SRC(obj)->priv->mutex)))

#define MEDIA_TYPE_DATA "type"

struct _KmsMixerSrcPriv {
	GMutex mutex;

	GstElement *adder;
};

enum {
	PROP_0,
};

static GstStaticPadTemplate audio_sink = GST_STATIC_PAD_TEMPLATE (
				"mixer_audio_sink%d",
				GST_PAD_SINK,
				GST_PAD_REQUEST,
				GST_STATIC_CAPS(
					"audio/x-raw-int, endianness=1234, "
					"signed=true, width=16, depth=16, "
					"rate=8000, channels=1")
);

static GstStaticPadTemplate audio_src = GST_STATIC_PAD_TEMPLATE (
						"audio_src%d",
						GST_PAD_SRC,
						GST_PAD_REQUEST,
						GST_STATIC_CAPS_ANY
);

G_DEFINE_TYPE(KmsMixerSrc, kms_mixer_src, KMS_TYPE_MEDIA_HANDLER_SRC)

static void
dispose_adder(KmsMixerSrc *self) {
	if (self->priv->adder == NULL) {
		g_object_unref(self->priv->adder);
		self->priv->adder = NULL;
	}
}

static void
found_media(GstElement* elem, guint prob, GstCaps* caps, KmsMixerSrc *self) {
	GstElement *adder;

	LOCK(self);
	adder = self->priv->adder;
	UNLOCK(self);

	if (adder == NULL)
		/* TODO: Possibly add a queue to avoid errors */
		return;

	gst_element_link(elem, adder);
}

static void
typefind_unlinked(GstPad *pad, GstPad *peer, gpointer not_used) {
	GstElement *elem;
	GstObject *parent;

	elem = gst_pad_get_parent_element(pad);

	if (elem == NULL)
		return;

	parent = gst_object_get_parent(GST_OBJECT(elem));

	if (parent == NULL) {
		g_object_unref(elem);
		return;
	}

	gst_bin_remove(GST_BIN(parent), elem);
	gst_element_set_state(elem, GST_STATE_NULL);

	g_object_unref(elem);
	g_object_unref(parent);
}

static GstPadLinkReturn
set_target_pad(KmsMixerSrc *self, GstPad *pad) {
	GstElement *typefind;
	GstPad *target_pad;
	GstPadLinkReturn ret;

	typefind = gst_element_factory_make("typefind", NULL);
	g_object_connect(typefind, "signal::have-type", found_media, self, NULL);

	if (typefind == NULL)
		return GST_PAD_LINK_WRONG_HIERARCHY;

	gst_element_set_state(typefind, GST_STATE_PLAYING);
	gst_bin_add(GST_BIN(self), typefind);

	target_pad = gst_element_get_static_pad(typefind, "sink");

	if (!gst_ghost_pad_set_target(GST_GHOST_PAD(pad), target_pad)) {
		gst_element_set_state(typefind, GST_STATE_NULL);
		gst_bin_remove(GST_BIN(self), typefind);
		ret = GST_PAD_LINK_REFUSED;
	} else {
		ret = GST_PAD_LINK_OK;
	}

	g_object_connect(target_pad, "signal::unlinked", typefind_unlinked,
								NULL, NULL);

	g_object_unref(target_pad);
	return ret;
}

static GstPadLinkReturn
link_pad(GstPad *pad, GstPad *peer) {
	GstElement *elem;
	KmsMixerSrc *self;
	GstPadLinkReturn ret;

	elem = gst_pad_get_parent_element(pad);
	if (elem == NULL)
		return GST_PAD_LINK_WRONG_HIERARCHY;

	self = KMS_MIXER_SRC(elem);

	ret = set_target_pad(self, pad);

	g_object_unref(elem);
	return ret;
}

static void
pad_unlinked(GstPad  *pad, GstPad  *peer, GstElement *elem) {
	gst_ghost_pad_set_target(GST_GHOST_PAD(pad), NULL);
	gst_element_release_request_pad(elem, pad);
}

static GstPad*
mixer_src_request_new_pad(GstElement *elem, GstPadTemplate *templ,
							const gchar *name) {
	GstPad *pad;
	gchar *new_name;

	if (g_strcmp0(GST_PAD_TEMPLATE_NAME_TEMPLATE(templ),
					audio_sink.name_template) != 0) {
		GstElementClass *p_class =
				GST_ELEMENT_CLASS(kms_mixer_src_parent_class);
		return p_class->request_new_pad(elem, templ, name);
	}

	if (name != NULL)
		new_name = g_strdup(name);
	else
		new_name = kms_utils_generate_pad_name(templ->name_template);

	pad = gst_ghost_pad_new_no_target_from_template(new_name, templ);
	g_free(new_name);

	gst_pad_set_active(pad, TRUE);
	gst_pad_set_link_function(pad, link_pad);
	g_object_connect(pad, "signal::unlinked", pad_unlinked, elem, NULL);

	gst_element_add_pad(elem, pad);
	return pad;
}

static void
create_audio_src(KmsMixerSrc *self) {
	GstElement *adder, *tee, *queue, *fake;
	GstPad *pad;

	adder = gst_element_factory_make("liveadder", NULL);
	tee = gst_element_factory_make("tee", NULL);
	queue = gst_element_factory_make("queue2", NULL);
	fake = kms_utils_create_fakesink(NULL);

	if (adder == NULL || tee == NULL || queue == NULL || fake == NULL) {
		if (adder != NULL)
			g_object_unref(adder);

		if (tee != NULL)
			g_object_unref(tee);

		if (queue != NULL)
			g_object_unref(queue);

		if (fake != NULL)
			g_object_unref(fake);
	}

	kms_utils_release_unlinked_pads(tee);
	kms_utils_release_unlinked_pads(adder);

	self->priv->adder = g_object_ref(adder);

	gst_element_set_state(adder, GST_STATE_PLAYING);
	gst_element_set_state(tee, GST_STATE_PLAYING);
	gst_element_set_state(queue, GST_STATE_PLAYING);
	gst_element_set_state(fake, GST_STATE_PLAYING);

	gst_bin_add_many(GST_BIN(self), adder, tee, queue, fake, NULL);
	gst_element_link_many(adder, tee, queue, fake, NULL);

	pad = gst_element_get_static_pad(adder, "src");
	kms_media_handler_src_set_raw_pad(KMS_MEDIA_HANDLER_SRC(self), pad, tee,
							KMS_MEDIA_TYPE_AUDIO);
}

static void
constructed(GObject *object) {
	KmsMixerSrc *self = KMS_MIXER_SRC(object);
	G_OBJECT_CLASS(kms_mixer_src_parent_class)->constructed(object);

	create_audio_src(self);
}

static void
dispose(GObject *object) {
	KmsMixerSrc *self = KMS_MIXER_SRC(object);

	kms_utils_remove_sink_pads(GST_ELEMENT(object));
	LOCK(self);
	dispose_adder(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_src_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerSrc *self = KMS_MIXER_SRC(object);

	g_mutex_clear(&self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_src_parent_class)->finalize(object);
}

static void
kms_mixer_src_class_init(KmsMixerSrcClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GstPadTemplate *templ;

	g_type_class_add_private(klass, sizeof(KmsMixerSrcPriv));

	object_class->finalize = finalize;
	object_class->dispose = dispose;
	object_class->constructed = constructed;

	GST_ELEMENT_CLASS(klass)->request_new_pad = mixer_src_request_new_pad;
	GST_ELEMENT_CLASS(klass)->numpadtemplates = 0;

	templ = gst_static_pad_template_get(&audio_sink);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);

	templ = gst_static_pad_template_get(&audio_src);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);
}

static void
kms_mixer_src_init(KmsMixerSrc *self) {
	self->priv = KMS_MIXER_SRC_GET_PRIVATE(self);

	g_mutex_init(&self->priv->mutex);
}
