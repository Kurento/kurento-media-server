#ifndef __KMS_MIXER_SRC_H__
#define __KMS_MIXER_SRC_H__

#include <glib-object.h>
#include <kms-core.h>

#define KMS_TYPE_MIXER_SRC			(kms_mixer_src_get_type())
#define KMS_MIXER_SRC(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MIXER_SRC, KmsMixerSrc))
#define KMS_IS_MIXER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MIXER_SRC))
#define KMS_MIXER_SRC_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsMixerSrcClass))
#define KMS_IS_MIXER_SRC_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MIXER_SRC))
#define KMS_MIXER_SRC_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MIXER_SRC, KmsMixerSrcClass))

G_BEGIN_DECLS

typedef struct _KmsMixerSrc		KmsMixerSrc;
typedef struct _KmsMixerSrcClass	KmsMixerSrcClass;
typedef struct _KmsMixerSrcPriv	KmsMixerSrcPriv;

struct _KmsMixerSrc {
	KmsMediaHandlerSrc parent_instance;

	/* instance members */

	KmsMixerSrcPriv *priv;
};

struct _KmsMixerSrcClass {
	KmsMediaHandlerSrcClass parent_class;

	/* class members */
};

GType kms_mixer_src_get_type (void);

G_END_DECLS

#endif /* __KMS_MIXER_SRC_H__ */
