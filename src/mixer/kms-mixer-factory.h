#ifndef __KMS_MIXER_FACTORY_H__
#define __KMS_MIXER_FACTORY_H__

#include <glib-object.h>
#include <kms-core.h>

/*
 * Type macros.
 */
#define KMS_TYPE_MIXER_FACTORY			(kms_mixer_factory_get_type ())
#define KMS_MIXER_FACTORY(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_MIXER_FACTORY, KmsMixerFactory))
#define KMS_IS_MIXER_FACTORY(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_MIXER_FACTORY))
#define KMS_MIXER_FACTORY_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MIXER_FACTORY, KmsMixerFactoryClass))
#define KMS_IS_MIXER_FACTORY_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MIXER_FACTORY))
#define KMS_MIXER_FACTORY_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MIXER_FACTORY, KmsMixerFactoryClass))

#define KMS_MIXER_FACTORY_ERROR (g_quark_from_string("KmsMixerFactoryError"))

typedef struct _KmsMixerFactory		KmsMixerFactory;
typedef struct _KmsMixerFactoryClass	KmsMixerFactoryClass;
typedef struct _KmsMixerFactoryPriv	KmsMixerFactoryPriv;

struct _KmsMixerFactory {
	GObject parent_instance;

	/* instance members */

	KmsMixerFactoryPriv *priv;
};

struct _KmsMixerFactoryClass {
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_MIXER_FACTORY */
GType kms_mixer_factory_get_type (void);

/*
 * Method definitions.
 */

void kms_mixer_factory_dispose(KmsMixerFactory *self);

void kms_mixer_factory_connect(KmsMixerFactory *self, KmsMixerFactory *other);

#endif /* __KMS_MIXER_FACTORY_H__ */
