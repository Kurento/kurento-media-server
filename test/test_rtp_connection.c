#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include "rtp-test-lib/rtp-test-lib.h"
#include <unistd.h>

#define CONNECTIONS 10

#define TESTS 6000

static void
test_connection() {
	KmsEndpoint *ep;
	KmsConnection *conn;
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

// 	ret = kms_connection_connect_to_remote(conn, NULL, &err);
// 	if (!ret && err != NULL) {
// 		g_printerr("error connecting: %s\n", err->message);
// 		g_error_free(err);
// 	}
// 	g_assert(ret);

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

	kms_init(&argc, &argv);

	for (i = 0; i < TESTS; i++) {
		test_connection();

		if (i == TESTS / 2) {
			sleep(5);
			g_print("mem: %d\n", get_data_memory());
		}
	}

	sleep(5);
	g_print("final mem: %d\n", get_data_memory());

	return 0;
}
