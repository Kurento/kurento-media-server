#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include "rtp-test-lib/rtp-test-lib.h"
#include <unistd.h>

#define CONNECTIONS 10

#define TESTS 6000

static void
get_ports(KmsSdpSession *session, gint *a_port, gint *v_port) {
	GValueArray *medias;
	gint i;

	g_object_get(session, "medias", &medias, NULL);

	for (i = 0; i < medias->n_values; i++) {
		GValue *val;
		KmsSdpMedia *media;
		KmsMediaType type;

		val = g_value_array_get_nth(medias, i);
		media = g_value_get_object(val);

		g_object_get(media, "type", &type, NULL);

		switch (type) {
			case KMS_MEDIA_TYPE_AUDIO:
				g_object_get(media, "port", a_port, NULL);
				g_assert(*a_port != 0);
				break;
			case KMS_MEDIA_TYPE_VIDEO:
				g_object_get(media, "port", v_port, NULL);
				g_assert(*v_port != 0);
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
	gint audio_port, video_port;

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

	get_ports(session, &audio_port, &video_port);

	g_object_unref(session);

	g_print("Audio port: %d\nVideo port:%d\n", audio_port, video_port);

	/* TODO: Send media to the port from a different pipeline */

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
	kms_init(&argc, &argv);

	test_connection();

	return 0;
}
