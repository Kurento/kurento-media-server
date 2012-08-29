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

#include "kms-test-connection.h"
#include <kms-core.h>

#define KMS_TEST_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_TEST_CONNECTION, KmsTestConnectionPriv))

/*
struct _KmsTestConnectionPriv {
};
*/

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsTestConnection, kms_test_connection,
				KMS_TYPE_CONNECTION,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init))

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	/*KmsTestConnection *self = KMS_TEST_CONNECTION(iface);*/

	g_print("No media handler factory defined yet\n");
	return NULL;
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
}

static void
kms_test_connection_class_init (KmsTestConnectionClass *klass) {
	/*
	g_type_class_add_private (klass, sizeof (KmsTestConnectionPriv));
	*/
}

static void
kms_test_connection_init (KmsTestConnection *self) {
	/*
	self->priv = KMS_TEST_CONNECTION_GET_PRIVATE(self);
	*/
}
