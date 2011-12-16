#include <kms-core.h>
#include "rtp/kms-rtp-endpoint.h"
#include "memory.h"
#include <glib.h>

#define CONNECTIONS 10

#define LOCALNAME "kms/rtp/1"

#define TESTS 6000

static KmsEndpoint*
create_endpoint() {
	KmsEndpoint *ep;
	gchar *name;

	name = g_strdup_printf(LOCALNAME);

	ep = g_object_new(KMS_TYPE_RTP_ENDPOINT, "localname", name, NULL);

	g_free(name);
	return ep;
}

static void
check_endpoint(KmsEndpoint *ep) {
	gchar *name;

	g_object_get(ep, "localname", &name, NULL);

	g_assert(g_strcmp0(name, LOCALNAME) == 0);

	g_free(name);
}

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

	ret = kms_connection_connect_to_remote(conn, NULL, &err);
	if (!ret && err != NULL) {
		g_printerr("error connecting: %s\n", err->message);
		g_error_free(err);
	}
	g_assert(ret);

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
	gint new_mem, mem = 0;

	g_type_init();

	for (i = 0; i < TESTS; i++) {
		test_connection();

		new_mem = get_data_memory();
		if (mem == 0)
			mem = new_mem;

		g_assert(mem == new_mem);

	}

	return 0;
}
