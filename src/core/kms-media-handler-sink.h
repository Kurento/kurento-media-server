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

#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_MEDIA_HANDLER_SINK_H__
#define __KMS_MEDIA_HANDLER_SINK_H__

#include <glib-object.h>
#include <gst/gst.h>

#define KMS_TYPE_MEDIA_HANDLER_SINK		(kms_media_handler_sink_get_type())
#define KMS_MEDIA_HANDLER_SINK(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSink))
#define KMS_IS_MEDIA_HANDLER_SINK(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_SINK))
#define KMS_MEDIA_HANDLER_SINK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SINK, KmsMediaHandlerSinkClass))
#define KMS_IS_MEDIA_HANDLER_SINK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MEDIA_HANDLER_SINK))
#define KMS_MEDIA_HANDLER_SINK_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSinkClass))

G_BEGIN_DECLS

typedef struct _KmsMediaHandlerSink		KmsMediaHandlerSink;
typedef struct _KmsMediaHandlerSinkClass	KmsMediaHandlerSinkClass;
typedef struct _KmsMediaHandlerSinkPriv		KmsMediaHandlerSinkPriv;

#define KMS_MEDIA_HANDLER_SINK_ERROR (g_quark_from_string("KmsMediaHandlerSinkError"))

enum {
	KMS_MEDIA_HANDLER_SINK_ERROR_UNLINK_ERROR
};

struct _KmsMediaHandlerSink {
	GstBin parent_instance;

	/* instance members */

	KmsMediaHandlerSinkPriv *priv;
};

struct _KmsMediaHandlerSinkClass {
	GstBinClass parent_class;

	/* class members */
};

G_END_DECLS

#include <kms-media-handler-src.h>

G_BEGIN_DECLS

GType kms_media_handler_sink_get_type (void);

gboolean kms_media_handler_sink_disconnect(KmsMediaHandlerSink *self,
					KmsMediaHandlerSrc *src,
					KmsMediaType type, GError **err);

void kms_media_handler_sink_terminate(KmsMediaHandlerSink *self);

G_END_DECLS

#endif /* __KMS_MEDIA_HANDLER_SINK_H__ */
