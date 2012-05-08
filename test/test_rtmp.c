/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <rtp/kms-rtp.h>
#include <kms-core.h>
#include "memory.h"
#include <glib.h>
#include <rtmp/kms-rtmp-endpoint.h>
#include <unistd.h>
#include "internal/kms-utils.h"

#define TESTS 5

#define URL "rtmp://localhost/videochat"

static gchar*
get_url(KmsMediaSpec *media) {
	gchar *url;

	g_assert(media->transport->__isset_rtmp);
	g_assert(media->transport->rtmp->__isset_url);
	g_assert(media->transport->rtmp->url != NULL);
	g_assert(media->transport->rtmp->__isset_publish);
	g_assert(media->transport->rtmp->publish != NULL);

	url = g_strdup_printf("%s/%s", media->transport->rtmp->url,
					media->transport->rtmp->publish);
	return url;
}

static gchar*
get_type_url(KmsSessionSpec *session, KmsMediaType type) {
	gchar *url;
	gint i;

	for (i = 0; i < session->medias->len; i++) {
		KmsMediaSpec *media;

		media = session->medias->pdata[i];

		if (g_hash_table_lookup(media->type, (gpointer) type)) {
			url = get_url(media);
			return url;
		}
	}

	return NULL;
}

static GstElement*
send_media(gchar *audio_url, gchar *video_url) {
	GstElement *pipe;
	GError *err = NULL;
	gchar *desc;

	desc = g_strdup_printf("audiotestsrc ! "
			"audio/x-raw-int,channels=1,rate=8000 ! "
			"ffenc_nellymoser ! queue2 ! flvmux name=mux ! queue2 ! "
			"rtmpsink location=%s videotestsrc pattern=18 ! "
			"video/x-raw-yuv,width=320,framerate=15/1 ! "
			"ffenc_flv ! "
			"queue2 ! flvmux ! queue2 ! rtmpsink location=%s",
							audio_url, video_url);
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

static KmsSessionSpec *
create_second_session() {
	KmsSessionSpec *session;
	KmsMediaSpec *amedia, *vmedia;

	session = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);

	// Add medias
	amedia = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
	amedia->direction = KMS_DIRECTION_SENDRECV;
	g_hash_table_insert(amedia->type, (gpointer) KMS_MEDIA_TYPE_AUDIO,
								(gpointer) 1);
	amedia->transport->__isset_rtmp = TRUE;
	g_free(amedia->transport->rtmp->url);
	amedia->transport->rtmp->__isset_url = TRUE;
	amedia->transport->rtmp->url = g_strdup(URL);
	g_free(amedia->transport->rtmp->publish);
	amedia->transport->rtmp->__isset_publish = TRUE;
	amedia->transport->rtmp->publish = g_strdup("test_audio");

	vmedia = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
	vmedia->direction = KMS_DIRECTION_SENDRECV;
	g_hash_table_insert(vmedia->type, (gpointer) KMS_MEDIA_TYPE_VIDEO,
								(gpointer) 1);
	vmedia->transport->__isset_rtmp = TRUE;
	g_free(vmedia->transport->rtmp->url);
	vmedia->transport->rtmp->__isset_url = TRUE;
	vmedia->transport->rtmp->url = g_strdup(URL);
	g_free(vmedia->transport->rtmp->publish);
	vmedia->transport->rtmp->__isset_publish = TRUE;
	vmedia->transport->rtmp->publish = g_strdup("test_video");

	g_ptr_array_add(session->medias, amedia);
	g_ptr_array_add(session->medias, vmedia);

	return session;
}

static KmsSessionSpec *
create_session() {
	KmsSessionSpec *session;
	KmsMediaSpec *amedia, *vmedia;

	session = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);

	// Add medias
	amedia = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
	amedia->transport->__isset_rtmp = TRUE;
	amedia->direction = KMS_DIRECTION_SENDRECV;
	g_hash_table_insert(amedia->type, (gpointer) KMS_MEDIA_TYPE_AUDIO,
								(gpointer) 1);

	vmedia = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
	vmedia->transport->__isset_rtmp = TRUE;
	vmedia->direction = KMS_DIRECTION_SENDRECV;
	g_hash_table_insert(vmedia->type, (gpointer) KMS_MEDIA_TYPE_VIDEO,
								(gpointer) 1);

	g_ptr_array_add(session->medias, amedia);
	g_ptr_array_add(session->medias, vmedia);

	return session;
}

static KmsEndpoint*
create_endpoint() {
	KmsEndpoint *ep;
	KmsSessionSpec *session;

	session = create_session();
	ep = g_object_new(KMS_TYPE_RTMP_ENDPOINT, "localname", "kms/rtmp/1",
						"local-spec", session, NULL);
	g_object_unref(session);

	return ep;
}

static void
test_connection() {
	KmsEndpoint *ep;
	KmsConnection *conn;
	KmsSessionSpec *session, *session2;
	GError *err = NULL;
	gboolean ret;
	GstElement *pipe;
	gchar *aurl, *vurl;

	ep = create_endpoint();

	conn = kms_endpoint_create_connection(ep, KMS_CONNECTION_TYPE_RTP,
					      &err);

	if (conn == NULL && err != NULL) {
		g_printerr("%s:%d: %s\n", __FILE__, __LINE__, err->message);
		g_error_free(err);
	}

	g_assert(conn != NULL);

	session2 = create_second_session();
	ret = kms_connection_connect_to_remote(conn, session2, FALSE, &err);
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
	aurl = get_type_url(session, KMS_MEDIA_TYPE_AUDIO);
	vurl = get_type_url(session, KMS_MEDIA_TYPE_VIDEO);

	g_object_unref(session);

	pipe = send_media(aurl, vurl);
	g_free(aurl);
	g_free(vurl);

	create_local_connections(ep);

	sleep(10);

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
