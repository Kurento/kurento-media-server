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

#ifndef __KMS_MIXER_SINK_H__
#define __KMS_MIXER_SINK_H__

#include <kms-core.h>
#include <glib-object.h>
#include <mixer/kms-mixer-src.h>

#define KMS_TYPE_MIXER_SINK		(kms_mixer_sink_get_type())
#define KMS_MIXER_SINK(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MIXER_SINK, KmsMixerSink))
#define KMS_IS_MIXER_SINK(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MIXER_SINK))
#define KMS_MIXER_SINK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsMixerSinkClass))
#define KMS_IS_MIXER_SINK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MIXER_SINK))
#define KMS_MIXER_SINK_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MIXER_SINK, KmsMixerSinkClass))

G_BEGIN_DECLS

typedef struct _KmsMixerSink		KmsMixerSink;
typedef struct _KmsMixerSinkClass	KmsMixerSinkClass;
typedef struct _KmsMixerSinkPriv	KmsMixerSinkPriv;

struct _KmsMixerSink {
	KmsMediaHandlerSink parent_instance;

	/* instance members */

	KmsMixerSinkPriv *priv;
};

struct _KmsMixerSinkClass {
	KmsMediaHandlerSinkClass parent_class;

	/* class members */
};

GType kms_mixer_sink_get_type (void);

void kms_mixer_sink_link(KmsMixerSink *sink, KmsMixerSrc *src);

G_END_DECLS

#endif /* __KMS_MIXER_SINK_H__ */
