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

#if !defined (__KMS_MIXER_H_INSIDE__)
#error "Only <mixer/kms-mixer.h> can be included directly."
#endif

#ifndef __KMS_MIXER_ENDPOINT_H__
#define __KMS_MIXER_ENDPOINT_H__

#include <glib-object.h>
#include <kms-core.h>

/*
 * Type macros.
 */
#define KMS_TYPE_MIXER_ENDPOINT			(kms_mixer_endpoint_get_type ())
#define KMS_MIXER_ENDPOINT(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_MIXER_ENDPOINT, KmsMixerEndpoint))
#define KMS_IS_MIXER_ENDPOINT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_MIXER_ENDPOINT))
#define KMS_MIXER_ENDPOINT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MIXER_ENDPOINT, KmsMixerEndpointClass))
#define KMS_IS_MIXER_ENDPOINT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_MIXER_ENDPOINT))
#define KMS_MIXER_ENDPOINT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_MIXER_ENDPOINT, KmsMixerEndpointClass))

#define KMS_MIXER_ENDPOINT_ERROR (g_quark_from_string("KmsMixerEndpointError"))

G_BEGIN_DECLS

typedef struct _KmsMixerEndpoint		KmsMixerEndpoint;
typedef struct _KmsMixerEndpointClass	KmsMixerEndpointClass;
typedef struct _KmsMixerEndpointPriv	KmsMixerEndpointPriv;

struct _KmsMixerEndpoint {
	KmsEndpoint parent_instance;

	/* instance members */

	KmsMixerEndpointPriv *priv;
};

struct _KmsMixerEndpointClass {
	KmsEndpointClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_MIXER_ENDPOINT */
GType kms_mixer_endpoint_get_type (void);

/*
 * Method definitions.
 */

G_END_DECLS

#endif /* __KMS_MIXER_ENDPOINT_H__ */
