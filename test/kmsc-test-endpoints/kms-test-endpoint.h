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

#ifndef __KMS_TEST_ENDPOINT_H__
#define __KMS_TEST_ENDPOINT_H__

#include <glib-object.h>
#include <kms-core.h>

/*
 * Type macros.
 */
#define KMS_TYPE_TEST_ENDPOINT			(kms_test_endpoint_get_type ())
#define KMS_TEST_ENDPOINT(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_TEST_ENDPOINT, KmsTestEndpoint))
#define KMS_IS_TEST_ENDPOINT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_TEST_ENDPOINT))
#define KMS_TEST_ENDPOINT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_TEST_ENDPOINT, KmsTestEndpointClass))
#define KMS_IS_TEST_ENDPOINT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_TEST_ENDPOINT))
#define KMS_TEST_ENDPOINT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_TEST_ENDPOINT, KmsTestEndpointClass))

typedef struct _KmsTestEndpoint		KmsTestEndpoint;
typedef struct _KmsTestEndpointClass	KmsTestEndpointClass;
typedef struct _KmsTestEndpointPriv	KmsTestEndpointPriv;

struct _KmsTestEndpoint {
	KmsEndpoint parent_instance;

	/* instance members */

	/*
	KmsTestEndpointPriv *priv;
	*/
};

struct _KmsTestEndpointClass {
	KmsEndpointClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_TEST_ENDPOINT */
GType kms_test_endpoint_get_type (void);

/*
 * Method definitions.
 */

#endif /* __KMS_TEST_ENDPOINT_H__ */
