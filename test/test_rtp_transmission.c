#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include "rtp-test-lib/rtp-test-lib.h"
#include <unistd.h>

#define CONNECTIONS 10

#define TESTS 60

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

static GstElement*
send_media(gint aport, gint vport) {
	GstElement *pipe;

	pipe = gst_pipeline_new(NULL);

	return pipe;
}

static void
test_connection() {
	KmsEndpoint *ep;
	KmsConnection *conn;
	KmsSdpSession *session, *session2;
	GError *err = NULL;
	gboolean ret;
	gint audio_port, video_port;
	GstElement *pipe;
	gchar *session_str;

	ep = create_endpoint();
	check_endpoint(ep);

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
					      &err);

	if (conn == NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}

	g_assert(conn != NULL);

	session2 = create_second_session();
	ret = kms_connection_connect_to_remote(conn, session2, &err);
	if (ret && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_object_unref(session2);
	g_assert(ret);

	if (g_object_class_find_property(G_OBJECT_GET_CLASS(conn),
		"descriptor") == NULL) {
		g_assert_not_reached();
	}

		g_object_get(conn, "descriptor", &session, NULL);
	g_assert(session != NULL);
	get_ports(session, &audio_port, &video_port);

	session_str = kms_sdp_session_to_string(session);
	g_print("session:\n%s\n", session_str);
	g_free(session_str);

	g_object_unref(session);

	g_print("Audio port: %d\nVideo port: %d\n", audio_port, video_port);

	pipe = send_media(audio_port, video_port);

	sleep(1);

	ret = kms_endpoint_delete_connection(ep, conn, &err);
	if (!ret && err != NULL) {
		g_printerr("error deleting: %s\n", err->message);
		g_error_free(err);
	}
	g_assert(ret);
	g_object_unref(conn);

	gst_element_set_state(pipe, GST_STATE_NULL);
	g_object_unref(pipe);

	check_endpoint(ep);
	g_object_unref(ep);
}

gint
main(gint argc, gchar **argv) {
	gint i;
	gint new_mem, mem = 0;

	kms_init(&argc, &argv);

	g_print("Initial memory: %d\n", get_data_memory());
	for (i = 0; i < TESTS; i++) {
		test_connection();
		sleep(1);
		new_mem = get_data_memory();
		g_print("%d\n", new_mem);

		if (i > 1)
			g_assert(mem == new_mem);
		mem = new_mem;
	}

	return 0;
}
