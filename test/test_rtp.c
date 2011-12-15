#include <string.h>
#include <stdlib.h>

#include "rtp/kms-rtp-endpoint.h"
#include <glib.h>

#define CONNECTIONS 100

#define LOCALNAME "kms/rtp/1"

#define PROC "/proc/self/status"

#define TESTS 1000

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
check_memory() {
	static gint mem = 0;
	gint new_mem;

	GIOChannel *chan;
	GError *err= NULL;
	GIOStatus st;
	char *line = NULL;
	char *vmdata = NULL;
	size_t len;

	chan = g_io_channel_new_file(PROC, "r", &err);
	if (chan == NULL && err != NULL) {
		g_printerr("%s:%d: %s", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}

	g_assert(chan != NULL);

	/* Read memory size data from /proc/pid/status */
	while (!vmdata) {

		st = g_io_channel_read_line(chan, &line, &len, NULL, &err);

		if (st != G_IO_STATUS_NORMAL && err != NULL) {
			g_printerr("%s:%d: %s", __FILE__, __LINE__, err->message);
			g_error_free(err);
		}

		g_assert(st == G_IO_STATUS_NORMAL);

		if (!strncmp(line, "VmData:", 7)) {
			vmdata = g_strdup(&line[7]);
		}

		g_free(line);
	}

	g_io_channel_shutdown(chan, FALSE, NULL);
	g_io_channel_unref(chan);

	/* Get rid of " kB\n"*/
	len = strlen(vmdata);
	vmdata[len - 4] = 0;

	new_mem = atoi(vmdata);

	if (mem == 0) {
		mem = new_mem;
	}
	g_assert(mem == new_mem);

	g_free(vmdata);
}

gint
main(gint argc, gchar **argv) {
	KmsEndpoint *ep;
	KmsConnection *conn;
	GError *err = NULL;
	gboolean ret;
	GSList *lc;
	gint i;

	g_type_init();

	for (i = 0; i < TESTS; i++) {

	ep = create_endpoint();
	check_endpoint(ep);

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
									NULL);

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

	check_memory();

	}

	return 0;
}
