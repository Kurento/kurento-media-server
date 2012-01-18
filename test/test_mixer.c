#include <mixer/kms-mixer.h>
#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include "rtp-test-lib/rtp-test-lib.h"
#include <unistd.h>
#include "internal/kms-utils.h"

#define TESTS 1

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
send_audio(gint port) {
	GstElement *apipe;
	GError *err = NULL;
	gchar *desc;

	desc = g_strdup_printf("audiotestsrc ! queue2 ! amrnbenc ! rtpamrpay ! "
					"application/x-rtp,encoding-name=AMR,"
					"clock-rate=8000,payload=96 ! "
					"udpsink host=127.0.0.1 port=%d", port);
	apipe = gst_parse_launch(desc, &err);
	if (!apipe && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_free(desc);

	g_assert(apipe != NULL);
	gst_element_set_state(apipe, GST_STATE_PLAYING);
	return apipe;
}

static GstElement*
send_video(gint port) {
	GstElement *vpipe;
	GError *err = NULL;
	gchar *desc;

	desc = g_strdup_printf("videotestsrc ! queue2 ! "
				"xvidenc max-bquant=0 bquant-ratio=0 motion=0 !"
				"rtpmp4vpay send-config=true ! "
				"application/x-rtp,encoding-name=MP4V-ES,"
				"clock-rate=90000,payload=106 ! "
				"udpsink host=127.0.0.1 port=%d", port);
	vpipe = gst_parse_launch(desc, &err);
	if (vpipe != NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}
	g_free(desc);

	g_assert(vpipe != NULL);

	gst_element_set_state(vpipe, GST_STATE_PLAYING);
	return vpipe;
}

static void
create_local_connections(KmsEndpoint *ep, KmsEndpoint *mixer) {
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

	lc2 = kms_endpoint_create_connection(mixer, KMS_CONNECTION_TYPE_LOCAL,
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

	ret = kms_connection_set_mode(lc1, KMS_CONNECTION_MODE_SENDRECV,
						KMS_MEDIA_TYPE_AUDIO, &err);
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

	g_object_unref(lc1);
	g_object_unref(lc2);
}

static void
test_mixer() {
	KmsEndpoint *ep, *mixer;
	KmsConnection *conn;
	KmsSdpSession *session, *session2;
	GError *err = NULL;
	gboolean ret;
	gint audio_port, video_port;
	GstElement *apipe, *vpipe;

	ep = create_endpoint();

	mixer = g_object_new(KMS_TYPE_MIXER_ENDPOINT, NULL);

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

	g_object_unref(session);

	apipe = send_audio(audio_port);
	vpipe = send_video(video_port);

	create_local_connections(ep, mixer);
	create_local_connections(ep, mixer);

	sleep(4);

	KMS_DEBUG_PIPE("before_finish");

	ret = kms_endpoint_delete_connection(ep, conn, &err);
	if (!ret && err != NULL) {
		g_printerr("error deleting: %s\n", err->message);
		g_error_free(err);
	}
	g_assert(ret);
	g_object_unref(conn);

	kms_endpoint_delete_all_connections(ep);
	kms_endpoint_delete_all_connections(mixer);

	KMS_DEBUG_PIPE("finished");

	gst_element_send_event(apipe, gst_event_new_eos());
	gst_element_set_state(apipe, GST_STATE_NULL);
	g_object_unref(apipe);

	gst_element_send_event(vpipe, gst_event_new_eos());
	gst_element_set_state(vpipe, GST_STATE_NULL);
	g_object_unref(vpipe);

	g_object_unref(ep);
	g_object_unref(mixer);
}

gint
main(gint argc, gchar **argv) {
	gint i;
	gint new_mem, mem2 = 0,  mem = 0;

	kms_init(&argc, &argv);

	g_print("Initial memory: %d\n", get_data_memory());
	for (i = 0; i < TESTS; i++) {
		test_mixer();
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
