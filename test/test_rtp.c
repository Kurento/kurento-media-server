#include "rtp/kms-rtp-endpoint.h"
#include <glib.h>

#define CONNECTIONS 100

#define LOCALNAME "kms/rtp/1"

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

gint
main(gint argc, gchar **argv) {
	KmsEndpoint *ep;
	KmsConnection *conn;
	GSList *lc;

	g_type_init();

	ep = create_endpoint();
	check_endpoint(ep);

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
									NULL);

	g_assert(conn != NULL);

	lc = create_local_connections(ep);

	delete_local_connections(ep, lc);

	kms_endpoint_delete_connection(ep, conn, NULL);
	g_object_unref(conn);

	check_endpoint(ep);

	g_object_unref(ep);

	return 0;
}
