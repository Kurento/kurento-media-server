#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include <rtmp/kms-rtmp-endpoint.h>
#include <rtmp/kms-rtmp-session.h>
#include <unistd.h>
#include "internal/kms-utils.h"

#define TESTS 30

static gchar*
get_url(KmsSdpSession *session) {
	KmsRtmpSession *rtmp_session;
	gchar *url;

	rtmp_session = kms_rtmp_session_create_from_sdp_session(session);

	url = kms_rtmp_session_get_url(rtmp_session, FALSE);
	g_object_unref(rtmp_session);

	return url;
}

static GstElement*
send_media(gchar *url) {
	GstElement *pipe;
	GError *err = NULL;
	gchar *desc;

	desc = g_strdup_printf("audiotestsrc ! "
			"audio/x-raw-int,channels=1,rate=8000 ! "
			"ffenc_nellymoser ! queue2 ! "
			"flvmux name=mux ! queue2 ! rtmpsink location=%s "
			"videotestsrc ! "
			"video/x-raw-yuv,width=320,framerate=15/1 ! "
			"ffenc_flv bitrate-tolerance=50000 ! queue2 ! mux.", url);
	pipe = gst_parse_launch(desc, &err);
	if (!pipe && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_free(desc);

	g_assert(pipe != NULL);
	gst_element_set_state(pipe, GST_STATE_PLAYING);

	return pipe;
}

static void
create_local_connections(KmsEndpoint *ep) {
	KmsConnection *lc1, *lc2;
	GError *err = NULL;
	gboolean ret;

	lc1 = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_LOCAL,
									&err);
	if (lc1 == NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(lc1 != NULL);

	lc2 = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_LOCAL,
					     &err);
	if (lc2 == NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(lc2 != NULL);

	ret = kms_connection_connect(lc1, lc2, KMS_MEDIA_TYPE_AUDIO, &err);
	if (!ret && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(ret);

	ret = kms_connection_connect(lc1, lc2, KMS_MEDIA_TYPE_VIDEO, &err);
	if (!ret && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(ret);

	ret = kms_connection_set_mode(lc1, KMS_CONNECTION_MODE_SENDRECV,
						KMS_MEDIA_TYPE_AUDIO, &err);
	if (!ret && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(ret);

	ret = kms_connection_set_mode(lc1, KMS_CONNECTION_MODE_SENDRECV,
				      KMS_MEDIA_TYPE_VIDEO, &err);
	if (!ret && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(ret);

	ret = kms_connection_set_mode(lc2, KMS_CONNECTION_MODE_SENDRECV,
				      KMS_MEDIA_TYPE_AUDIO, &err);
	if (!ret && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(ret);

	ret = kms_connection_set_mode(lc2, KMS_CONNECTION_MODE_SENDRECV,
				      KMS_MEDIA_TYPE_VIDEO, &err);
	if (!ret && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_assert(ret);

	g_object_unref(lc1);
	g_object_unref(lc2);
}

static KmsSdpSession *
create_second_session() {
	KmsSdpSession *session;
	KmsRtmpSession *rtmp_session;

	rtmp_session = kms_rtmp_session_create_from_string(
				"url=rtmp://localhost/videochat;"
				"offerer=test_stream;w=320;h=240;fps=15/1");

	session = kms_rtmp_session_get_sdp_session(rtmp_session);
	g_object_unref(rtmp_session);

	return session;
}

static KmsEndpoint*
create_endpoint() {
	KmsEndpoint *ep;
	KmsRtmpSession *session;

	session = kms_rtmp_session_create_from_string("w=320;h=240;fps=15/1");
	ep = g_object_new(KMS_TYPE_RTMP_ENDPOINT, "localname", "kms/rtmp/1",
						"local-spec", session, NULL);

	g_object_unref(session);

	return ep;
}

static void
test_connection() {
	KmsEndpoint *ep;
	KmsConnection *conn;
	KmsSdpSession *session, *session2;
	GError *err = NULL;
	gboolean ret;
	GstElement *pipe;
	gchar *url;

	ep = create_endpoint();

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
					      &err);

	if (conn == NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}

	g_assert(conn != NULL);

	session2 = create_second_session();
	ret = kms_connection_connect_to_remote(conn, session2, &err);
	if (!ret && err != NULL) {
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
	url = get_url(session);

	g_object_unref(session);

	pipe = send_media(url);
	g_free(url);

	create_local_connections(ep);

	sleep(6);

	KMS_DEBUG_PIPE("before_finish");

	ret = kms_endpoint_delete_connection(ep, conn, &err);
	if (!ret && err != NULL) {
		g_printerr("error deleting: %s\n", err->message);
		g_error_free(err);
	}
	g_assert(ret);
	g_object_unref(conn);

	kms_endpoint_delete_all_connections(ep);

	KMS_DEBUG_PIPE("finished");

	gst_element_send_event(pipe, gst_event_new_eos());
	gst_element_set_state(pipe, GST_STATE_NULL);
	g_object_unref(pipe);

	g_object_unref(ep);
}

gint
main(gint argc, gchar **argv) {
	gint i;
	gint new_mem, mem2 = 0,  mem = 0;

	kms_init(&argc, &argv);

	g_print("Initial memory: %d\n", get_data_memory());
	for (i = 0; i < TESTS; i++) {
		test_connection();
		sleep(2);
		new_mem = get_data_memory();
		g_print("%d - %d\n", i, new_mem);

		if (i > 2)
			g_warn_if_fail(mem >= new_mem || mem2 >= new_mem ||
								mem2 >= mem);
		mem2 = mem;
		mem = new_mem;
	}

	return 0;
}
