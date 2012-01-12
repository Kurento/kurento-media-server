#include "rtp-test-lib.h"
#include <rtp/kms-rtp.h>
#include <kms-core.h>

#define LOCALNAME "kms/rtp/1"

#define ADDR "127.0.0.1"
#define SESSION_NAME "kurento session"
#define SESSION_ID 12345
#define SESSION_VERSION 54321
#define SESSION_REMOTE_HANDLER "127.0.0.1"

#define ADDR_2 "127.0.0.1"
#define SESSION_NAME_2 "kurento session 2"
#define SESSION_ID_2 23456
#define SESSION_VERSION_2 65432
#define SESSION_REMOTE_HANDLER_2 "127.0.0.1"

#define PORT_AUDIO 2000
#define PORT_VIDEO 3000
#define PORT_AUDIO_2 5000
#define PORT_VIDEO_2 6000
#define BANDWIDTH 0

static GValueArray*
create_audio_payloads(gboolean second) {
	GValueArray *payloads;

	payloads = g_value_array_new(2);

	{
		KmsSdpPayload *pay;
		GValue vpay = G_VALUE_INIT;

		pay = g_object_new(KMS_TYPE_SDP_PAYLOAD, "name", "AMR",
							"payload", 96,
							"clockrate", 8000,
							NULL);

		g_value_init(&vpay, KMS_TYPE_SDP_PAYLOAD);
		g_value_take_object(&vpay, pay);
		g_value_array_append(payloads, &vpay);
		g_value_unset(&vpay);
	}

	if (!second) {
		KmsSdpPayload *pay;
		GValue vpay = G_VALUE_INIT;

		pay = g_object_new(KMS_TYPE_SDP_PAYLOAD, "name", "GSM",
							"payload", 97,
							"clockrate", 8000,
							NULL);

		g_value_init(&vpay, KMS_TYPE_SDP_PAYLOAD);
		g_value_take_object(&vpay, pay);
		g_value_array_append(payloads, &vpay);
		g_value_unset(&vpay);
	}

	return payloads;
}

static GValueArray*
create_video_payloads(gboolean second) {
	GValueArray *payloads;

	payloads = g_value_array_new(2);

	{
		KmsSdpPayload *pay;
		GValue vpay = G_VALUE_INIT;
		gint pay_id;

		if (second)
			pay_id = 106;
		else
			pay_id = 101;

		pay = g_object_new(KMS_TYPE_SDP_PAYLOAD, "name", "MP4V-ES",
							"payload", pay_id,
							"clockrate", 90000,
							NULL);

		g_value_init(&vpay, KMS_TYPE_SDP_PAYLOAD);
		g_value_take_object(&vpay, pay);
		g_value_array_append(payloads, &vpay);
		g_value_unset(&vpay);
	}

	{
		KmsSdpPayload *pay;
		GValue vpay = G_VALUE_INIT;
		gint pay_id;

		if (second)
			pay_id = 105;
		else
			pay_id = 100;

		pay = g_object_new(KMS_TYPE_SDP_PAYLOAD, "name", "H263-1998",
							"payload", pay_id,
							"clockrate", 90000,
							NULL);

		g_value_init(&vpay, KMS_TYPE_SDP_PAYLOAD);
		g_value_take_object(&vpay, pay);
		g_value_array_append(payloads, &vpay);
		g_value_unset(&vpay);
	}

	return payloads;
}

static GValueArray*
create_medias(gboolean second) {
	GValueArray *payloads;
	GValueArray *medias;
	gint port;
	KmsMediaType type;
	KmsSdpMode mode;
	glong bandwidth;

	medias = g_value_array_new(2);

	{
		KmsSdpMedia *media;
		GValue vmedia = G_VALUE_INIT;

		payloads = create_audio_payloads(second);

		if (second)
			port = PORT_AUDIO_2;
		else
			port = PORT_AUDIO;
		type = KMS_MEDIA_TYPE_AUDIO;
		mode = KMS_SDP_MODE_SENDRECV;
		bandwidth = BANDWIDTH;

		media = g_object_new(KMS_TYPE_SDP_MEDIA, "port", port,
							"bandwidth", bandwidth,
							"mode", mode,
							"type", type,
							"payloads", payloads,
							NULL);

		g_value_init(&vmedia, KMS_TYPE_SDP_MEDIA);
		g_value_take_object(&vmedia, media);
		g_value_array_append(medias, &vmedia);
		g_value_unset(&vmedia);

		g_value_array_free(payloads);
	}

	{
		KmsSdpMedia *media;
		GValue vmedia = G_VALUE_INIT;

		payloads = create_video_payloads(second);

		if (second)
			port = PORT_VIDEO_2;
		else
			port = PORT_VIDEO;
		type = KMS_MEDIA_TYPE_VIDEO;
		mode = KMS_SDP_MODE_SENDRECV;
		bandwidth = BANDWIDTH;

		media = g_object_new(KMS_TYPE_SDP_MEDIA, "port", port,
							"bandwidth", bandwidth,
							"mode", mode,
							"type", type,
							"payloads", payloads,
							NULL);

		g_value_init(&vmedia, KMS_TYPE_SDP_MEDIA);
		g_value_take_object(&vmedia, media);
		g_value_array_append(medias, &vmedia);
		g_value_unset(&vmedia);

		g_value_array_free(payloads);
	}

	return medias;
}

static KmsSdpSession*
create_session() {
	KmsSdpSession *session;
	GValueArray *medias;

	medias = create_medias(FALSE);

	session = g_object_new(KMS_TYPE_SDP_SESSION, "medias", medias,
				"address", ADDR,
				"name", SESSION_NAME,
				"id", SESSION_ID,
				"version", SESSION_VERSION,
				"remote-handler", SESSION_REMOTE_HANDLER,
				NULL);

	g_value_array_free(medias);

	return session;
}

KmsSdpSession*
create_second_session() {
	KmsSdpSession *session;
	GValueArray *medias;

	medias = create_medias(TRUE);

	session = g_object_new(KMS_TYPE_SDP_SESSION, "medias", medias,
				"address", ADDR_2,
				"name", SESSION_NAME_2,
				"id", SESSION_ID_2,
				"version", SESSION_VERSION_2,
				"remote-handler", SESSION_REMOTE_HANDLER_2,
				NULL);

	g_value_array_free(medias);

	return session;
}

KmsEndpoint*
create_endpoint() {
	KmsEndpoint *ep;
	KmsSdpSession *session;
	gchar *name;

	name = g_strdup_printf(LOCALNAME);
	session = create_session();

	ep = g_object_new(KMS_TYPE_RTP_ENDPOINT, "localname", name, NULL);

	g_object_set(ep, "local-spec", session, NULL);

	g_object_unref(session);
	g_free(name);
	return ep;
}

void
check_endpoint(KmsEndpoint *ep) {
	gchar *name;

	g_object_get(ep, "localname", &name, NULL);

	g_assert(g_strcmp0(name, LOCALNAME) == 0);

	g_free(name);
}