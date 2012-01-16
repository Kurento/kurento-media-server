#ifndef __KMS_MIXER_MANAGER_H__
#define __KMS_MIXER_MANAGER_H__

#include <glib-object.h>
#include <kms-core.h>

/*
 * Type macros.
 */
#define KMS_TYPE_MIXER_MANAGER			(kms_mixer_manager_get_type ())
#define KMS_MIXER_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_MIXER_MANAGER, KmsMixerManager))
#define KMS_IS_MIXER_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_MIXER_MANAGER))
#define KMS_MIXER_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MIXER_MANAGER, KmsMixerManagerClass))
#define KMS_IS_MIXER_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MIXER_MANAGER))
#define KMS_MIXER_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MIXER_MANAGER, KmsMixerManagerClass))

#define KMS_MIXER_MANAGER_ERROR (g_quark_from_string("KmsMixerManagerError"))

typedef struct _KmsMixerManager		KmsMixerManager;
typedef struct _KmsMixerManagerClass	KmsMixerManagerClass;
typedef struct _KmsMixerManagerPriv	KmsMixerManagerPriv;

struct _KmsMixerManager {
	GObject parent_instance;

	/* instance members */

	KmsMixerManagerPriv *priv;
};

struct _KmsMixerManagerClass {
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_MIXER_MANAGER */
GType kms_mixer_manager_get_type (void);

/*
 * Method definitions.
 */

#endif /* __KMS_MIXER_MANAGER_H__ */
