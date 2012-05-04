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

#ifndef __KMS_RESOURCE_H__
#define __KMS_RESOURCE_H__

#include <glib-object.h>
#include "kms-enums.h"
#include "kms-sdp-session.h"

/*
 * Type macros.
 */
#define KMS_TYPE_RESOURCE		(kms_resource_get_type())
#define KMS_RESOURCE(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_RESOURCE, KmsResource))
#define KMS_IS_RESOURCE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_RESOURCE))
#define KMS_RESOURCE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_INTERFACE((obj), KMS_TYPE_RESOURCE, KmsResourceClass))

G_BEGIN_DECLS

typedef struct _KmsResource		KmsResource;
typedef struct _KmsResourceInterface	KmsResourceInterface;

struct _KmsResourceInterface {
	GTypeInterface parent_class;

	/* class members */
};

/* used by KMS_TYPE_RESOURCE */
GType kms_resource_get_type (void);

/*
 * Method definitions.
 */

G_END_DECLS

#endif /* __KMS_RESOURCE_H__ */
