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

#include "rtp-test-lib.h"
#include <rtp/kms-rtp.h>
#include <kms-core.h>

#define LOCALNAME "kms/rtp/1"

#define ADDR "127.0.0.1"
#define SESSION_NAME "kurento session"
#define SESSION_ID "12345"
#define SESSION_VERSION "54321"
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

static GPtrArray*
create_audio_payloads(gboolean second) {
	GPtrArray *payloads;

	payloads = g_ptr_array_new_full(2, g_object_unref);

	{
		KmsPayload *pay;

		pay = g_object_new(KMS_TYPE_PAYLOAD, NULL);

		if (pay->rtp != NULL) {
			pay->rtp = g_object_new(KMS_TYPE_PAYLOAD_RTP, NULL);
		}

		pay->__isset_rtp = TRUE;

		if (pay->rtp->codecName != NULL)
			g_free(pay->rtp->codecName);
		pay->rtp->codecName = g_strdup("AMR");
		pay->rtp->id = 96;
		pay->rtp->clockRate = 8000;
		pay->rtp->bitrate = BANDWIDTH;
		pay->rtp->__isset_bitrate = TRUE;

		g_ptr_array_add(payloads, pay);
	}

	if (!second) {
		KmsPayload *pay;

		pay = g_object_new(KMS_TYPE_PAYLOAD, NULL);

		if (pay->rtp != NULL) {
			pay->rtp = g_object_new(KMS_TYPE_PAYLOAD_RTP, NULL);
		}

		pay->__isset_rtp = TRUE;

		if (pay->rtp->codecName != NULL)
			g_free(pay->rtp->codecName);
		pay->rtp->codecName = g_strdup("GSM");
		pay->rtp->id = 97;
		pay->rtp->clockRate = 8000;
		pay->rtp->bitrate = BANDWIDTH;
		pay->rtp->__isset_bitrate = TRUE;

		g_ptr_array_add(payloads, pay);
	}

	return payloads;
}


static GPtrArray*
create_video_payloads(gboolean second) {
	GPtrArray *payloads;

	payloads = g_ptr_array_new_full(2, g_object_unref);

	{
		KmsPayload *pay;
		gint pay_id;

		if (second)
			pay_id = 106;
		else
			pay_id = 101;

		pay = g_object_new(KMS_TYPE_PAYLOAD, NULL);

		if (pay->rtp != NULL) {
			pay->rtp = g_object_new(KMS_TYPE_PAYLOAD_RTP, NULL);
		}

		pay->__isset_rtp = TRUE;

		if (pay->rtp->codecName != NULL)
			g_free(pay->rtp->codecName);
		pay->rtp->codecName = g_strdup("MP4V-ES");
		pay->rtp->id = pay_id;
		pay->rtp->clockRate = 90000;
		pay->rtp->bitrate = BANDWIDTH;
		pay->rtp->__isset_bitrate = TRUE;

		g_ptr_array_add(payloads, pay);
	}

	{
		KmsPayload *pay;
		gint pay_id;

		if (second)
			pay_id = 105;
		else
			pay_id = 100;

		pay = g_object_new(KMS_TYPE_PAYLOAD, NULL);

		if (pay->rtp != NULL) {
			pay->rtp = g_object_new(KMS_TYPE_PAYLOAD_RTP, NULL);
		}

		pay->__isset_rtp = TRUE;

		if (pay->rtp->codecName != NULL)
			g_free(pay->rtp->codecName);
		pay->rtp->codecName = g_strdup("H263-1998");
		pay->rtp->id = pay_id;
		pay->rtp->clockRate = 90000;
		pay->rtp->bitrate = BANDWIDTH;
		pay->rtp->__isset_bitrate = TRUE;

		g_ptr_array_add(payloads, pay);
	}

	return payloads;
}

static GPtrArray*
create_medias(gboolean second) {
	GPtrArray *payloads;
	GPtrArray *medias;
	gint port;
	gchar *address;
	KmsMediaType type;
	KmsDirection mode;

	if (second) {
		address = ADDR_2;
	} else {
		address = ADDR;
	}

	medias = g_ptr_array_new_full(2, g_object_unref);

	{
		KmsMediaSpec *media;

		payloads = create_audio_payloads(second);

		if (second)
			port = PORT_AUDIO_2;
		else
			port = PORT_AUDIO;
		type = KMS_MEDIA_TYPE_AUDIO;
		mode = KMS_DIRECTION_SENDRECV;

		media = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);

		media->transport->__isset_rtp = TRUE;
		media->transport->rtp->port = port;
		media->transport->rtp->__isset_port = TRUE;
		media->transport->rtp->address = g_strdup(address);
		media->transport->rtp->__isset_address = TRUE;

		g_ptr_array_free(media->payloads, TRUE);
		media->payloads = payloads;
		media->direction = mode;

		g_hash_table_insert(media->type, GINT_TO_POINTER(type),
								(gpointer) 1);

		g_ptr_array_add(medias, media);
	}

	{
		KmsMediaSpec *media;

		payloads = create_video_payloads(second);

		if (second)
			port = PORT_VIDEO_2;
		else
			port = PORT_VIDEO;

		type = KMS_MEDIA_TYPE_VIDEO;
		mode = KMS_DIRECTION_SENDRECV;

		media = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);

		media->transport->__isset_rtp = TRUE;
		media->transport->rtp->port = port;
		media->transport->rtp->__isset_port = TRUE;
		media->transport->rtp->address = g_strdup(address);
		media->transport->rtp->__isset_address = TRUE;

		g_ptr_array_free(media->payloads, TRUE);
		media->payloads = payloads;
		media->direction = mode;

		g_hash_table_insert(media->type, GINT_TO_POINTER(type),
								(gpointer) 1);

		g_ptr_array_add(medias, media);
	}

	return medias;
}

static KmsSessionSpec*
create_session() {
	KmsSessionSpec *session;
	GPtrArray *medias;

	medias = create_medias(FALSE);

	session = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);

	g_ptr_array_free(session->medias, TRUE);
	session->medias = medias;

	session->id = g_strdup(SESSION_ID);

	session->version = g_strdup(SESSION_VERSION);
	session->__isset_version = TRUE;

	return session;
}

KmsSessionSpec*
create_second_session() {
	KmsSessionSpec *session;
	GPtrArray *medias;

	medias = create_medias(TRUE);

	session = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);

	g_ptr_array_free(session->medias, TRUE);
	session->medias = medias;

	session->id = g_strdup(SESSION_ID);

	session->version = g_strdup(SESSION_VERSION);
	session->__isset_version = TRUE;

	return session;
}

KmsEndpoint*
create_endpoint() {
	KmsEndpoint *ep;
	KmsSessionSpec *session;
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