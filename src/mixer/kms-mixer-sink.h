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

#endif /* __KMS_MIXER_SINK_H__ */
