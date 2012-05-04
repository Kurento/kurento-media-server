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

G_BEGIN_DECLS

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

G_END_DECLS

#endif /* __KMS_MIXER_MANAGER_H__ */
