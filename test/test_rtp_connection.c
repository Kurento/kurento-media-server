#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include "rtp-test-lib/rtp-test-lib.h"
#include <unistd.h>

#define CONNECTIONS 10

#define TESTS 6000

static void
check_ports(KmsSdpSession *session) {
	GValueArray *medias;
	gint i;

	g_object_get(session, "medias", &medias, NULL);

	for (i = 0; i < medias->n_values; i++) {
		GValue *val;
		KmsSdpMedia *media;
		KmsMediaType type;
		gint port;

		val = g_value_array_get_nth(medias, i);
		media = g_value_get_object(val);

		g_object_get(media, "type", &type, NULL);

		switch (type) {
			case KMS_MEDIA_TYPE_AUDIO:
			case KMS_MEDIA_TYPE_VIDEO:
				g_object_get(media, "port", &port, NULL);
				g_assert(port != 0);
				break;
			default:
				break;
		}
	}

	g_value_array_free(medias);
}

static void
test_connection() {
	KmsEndpoint *ep;
	KmsConnection *conn;
	KmsSdpSession *session;
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

	g_object_get(conn, "descriptor", &session, NULL);

	g_assert(session != NULL);

	check_ports(session);

	g_object_unref(session);

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
