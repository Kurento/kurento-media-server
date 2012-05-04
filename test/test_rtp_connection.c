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

#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include "rtp-test-lib/rtp-test-lib.h"
#include <unistd.h>

#define CONNECTIONS 10

#define TESTS 6000

static void
check_ports(KmsSessionSpec *session) {
	GPtrArray *medias;
	gint i;

	medias = session->medias;

	for (i = 0; i < medias->len; i++) {
		KmsMediaSpec *media;

		media = medias->pdata[i];

		g_assert(media->transport->__isset_rtp);
		g_assert(media->transport->rtp->port != 0);
	}
}

static void
test_connection() {
	KmsEndpoint *ep;
	KmsConnection *conn;
	KmsSessionSpec *session, *second_spec;
	GError *err = NULL;
	gboolean ret;

	ep = create_endpoint();
	check_endpoint(ep);

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
									&err);

	if (conn == NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}

	g_assert(conn != NULL);

	if (g_object_class_find_property(G_OBJECT_GET_CLASS(conn),
							"descriptor") == NULL) {
		g_assert_not_reached();
	}

	second_spec = create_second_session();
	ret = kms_connection_connect_to_remote(conn, second_spec, FALSE, &err);
	if (!ret && err != NULL) {
		g_printerr("error connecting: %s\n", err->message);
		g_error_free(err);
	}
	g_object_unref(second_spec);
	g_assert(ret);

	g_object_get(conn, "descriptor", &session, NULL);
	g_assert(session != NULL);
	check_ports(session);
	g_object_unref(session);

	ret = kms_endpoint_delete_connection(ep, conn, &err);
	if (!ret && err != NULL) {
		g_printerr("error deleting: %s\n", err->message);
		g_error_free(err);
	}
	g_assert(ret);
	g_object_unref(conn);

	check_endpoint(ep);
	g_object_unref(ep);
}

gint
main(gint argc, gchar **argv) {
	gint i;
	gint mem, mem2, final_mem;

	kms_init(&argc, &argv);

	get_data_memory();

	for (i = 0; i < TESTS; i++) {
		test_connection();
		sleep(0);

		if (i == TESTS / 3) {
			sleep(2);
			mem = get_data_memory();
		} else if (i == (TESTS / 3) * 2) {
			sleep(2);
			mem2 = get_data_memory();
		}
	}

	sleep(2);
	final_mem = get_data_memory();
	g_print("mem: %d\n", mem);
	g_print("mem2: %d\n", mem2);
	g_print("final mem: %d\n", final_mem);
	g_assert((mem == final_mem) || (mem2 == final_mem) || (mem == mem2));

	return 0;
}
