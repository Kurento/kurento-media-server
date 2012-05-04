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

#ifndef __KMS_LOCAL_CONNECTION_H__
#define __KMS_LOCAL_CONNECTION_H__

#include <glib-object.h>
#include "kms-connection.h"

/*
 * Type macros.
 */
#define KMS_TYPE_LOCAL_CONNECTION		(kms_local_connection_get_type ())
#define KMS_LOCAL_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_LOCAL_CONNECTION, KmsLocalConnection))
#define KMS_IS_LOCAL_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_LOCAL_CONNECTION))
#define KMS_LOCAL_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_LOCAL_CONNECTION, KmsLocalConnectionClass))
#define KMS_IS_LOCAL_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_LOCAL_CONNECTION))
#define KMS_LOCAL_CONNECTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_LOCAL_CONNECTION, KmsLocalConnectionClass))

G_BEGIN_DECLS

typedef struct _KmsLocalConnection		KmsLocalConnection;
typedef struct _KmsLocalConnectionClass		KmsLocalConnectionClass;
typedef struct _KmsLocalConnectionPriv		KmsLocalConnectionPriv;

struct _KmsLocalConnection {
	KmsConnection parent_instance;

	/* instance members */

	KmsLocalConnectionPriv *priv;
};

struct _KmsLocalConnectionClass {
	KmsConnectionClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_LOCAL_CONNECTION */
GType kms_local_connection_get_type (void);

/*
 * Method definitions.
 */

G_END_DECLS

#endif /* __KMS_LOCAL_CONNECTION_H__ */
