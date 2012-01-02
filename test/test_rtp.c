#include <rtp/kms-rtp.h>
#include "memory.h"
#include <glib.h>
#include "rtp-test-lib/rtp-test-lib.h"

#define CONNECTIONS 10

#define LOCALNAME "kms/rtp/1"

#define TESTS 6000

static GSList*
create_local_connections(KmsEndpoint *ep) {
	KmsConnection *lc;
	GSList *list = NULL;
	GError *err = NULL;
	gint i;

	for (i = 0; i < CONNECTIONS; i++) {
		lc = kms_endpoint_create_connection(ep,
					KMS_CONNECTION_TYPE_LOCAL, &err);

		if (lc == NULL && err != NULL) {
			g_printerr("error: %s\n", err->message);
			g_error_free(err);
		}
		g_assert(lc != NULL);
		list = g_slist_prepend(list, lc);
	}

	return list;
}

static void
delete_local_connections(KmsEndpoint *ep, GSList *lc) {
	KmsConnection *conn;
	GError *err = NULL;
	GSList *l = lc;
	gboolean ret;

	while (l != NULL) {
		conn = l->data;
		ret = kms_endpoint_delete_connection(ep, conn, &err);

		if (!ret && err != NULL) {
			g_printerr("error: %s\n", err->message);
			g_error_free(err);
		}

		g_object_unref(conn);
		g_assert(ret);
		l = l->next;
	}

	g_slist_free(lc);
}

static void
test_endpoint() {
	KmsEndpoint *ep;
	KmsConnection *conn;
	GError *err = NULL;
	gboolean ret;
	GSList *lc;

	ep = create_endpoint();
	check_endpoint(ep);

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
									&err);

	if (conn == NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}

	g_assert(conn != NULL);

	lc = create_local_connections(ep);

	delete_local_connections(ep, lc);

	ret = kms_endpoint_delete_connection(ep, conn, &err);
	if (!ret && err != NULL) {
		g_printerr("error deleting: %s\n", err->message);
		g_error_free(err);
	}
	g_assert(ret);
	g_object_unref(conn);

	check_endpoint(ep);

	/* Second test with delete all */

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
									NULL);

	g_assert(conn != NULL);

	lc = create_local_connections(ep);
	g_slist_free_full(lc, g_object_unref);
	g_object_unref(conn);

	check_endpoint(ep);

	kms_endpoint_delete_all_connections(ep);

	check_endpoint(ep);
	g_object_unref(ep);
}

gint
main(gint argc, gchar **argv) {
	gint i;

	kms_init(&argc, &argv);

	for (i = 0; i < TESTS; i++) {
		test_endpoint();
	}

	return 0;
}
