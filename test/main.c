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

#include "kms-test-endpoint.h"
#include <glib.h>

int main(int argc, char **argv) {
	GObject *ep;
	GValue val = G_VALUE_INIT;
	GError *err = NULL;
	KmsConnection *conn = NULL;
	KmsConnection *local = NULL;

	kms_init(&argc, &argv);

	ep = g_object_new(KMS_TYPE_TEST_ENDPOINT, "localname", "test_ep", NULL);

	if (ep == NULL) {
		g_print("Create endpont is: NULL\n");
		return 1;
	}

	g_value_init(&val, G_TYPE_STRING);
	g_object_get_property(ep, "localname", &val);
	g_print("Created ep with localname: %s\n", g_value_get_string(&val));
	g_value_unset(&val);


	conn = kms_endpoint_create_connection(KMS_ENDPOINT(ep),
					KMS_CONNECTION_TYPE_RTP, &err);

	if (conn == NULL) {
		g_print("Connection can not be created: %s\n", err->message);
		g_error_free(err);
		goto end;
	}

	g_value_init(&val, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(conn), "id", &val);
	g_print("Created rtp connection with id: %s\n",
						g_value_get_string(&val));
	g_value_unset(&val);

	g_value_init(&val, G_TYPE_OBJECT);
	g_object_get_property(G_OBJECT(conn), "endpoint", &val);
	g_print("Connection endpoint: %p - %p\n", g_value_get_object(&val),
		(gpointer) ep);
	g_value_unset(&val);

	local = kms_endpoint_create_connection(KMS_ENDPOINT(ep),
					      KMS_CONNECTION_TYPE_LOCAL, &err);

	if (local == NULL) {
		g_print("Connection can not be created: %s\n", err->message);
		g_error_free(err);
		goto end;
	}

	g_value_init(&val, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(local), "id", &val);
	g_print("Created local connection with id: %s\n",
		g_value_get_string(&val));
	g_value_unset(&val);

	g_value_init(&val, KMS_TYPE_ENDPOINT);
	g_object_get_property(G_OBJECT(local), "endpoint", &val);
	g_print("Local endpoint: %p - %p\n", g_value_get_object(&val),
								(gpointer) ep);
	g_value_unset(&val);

	if (!kms_endpoint_delete_connection(KMS_ENDPOINT(ep), local, &err)) {
		g_printerr("Connection can not be deleted: %s", err->message);
		g_error_free(err);
		goto end;
	}
	g_object_unref(local);

	if (!kms_endpoint_delete_connection(KMS_ENDPOINT(ep), conn, &err)) {
		g_printerr("Connection can not be deleted: %s", err->message);
		g_error_free(err);
		goto end;
	}
	g_object_unref(conn);

end:
	g_object_unref(ep);

	return 0;
}
