#include "rtp-test-lib.h"
#include <rtp/kms-rtp-endpoint.h>
#include <kms-core.h>

#define LOCALNAME "kms/rtp/1"

#define ADDR "127.0.0.1"
#define SESSION_NAME "kurento session"
#define SESSION_ID 12345
#define SESSION_VERSION 54321
#define SESSION_REMOTE_HANDLER "127.0.0.1"

static KmsSdpSession*
create_session() {
	KmsSdpSession *session;
	GValueArray *medias;

// 	medias = create_medias();
	medias = g_value_array_new(0);

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

KmsEndpoint*
create_endpoint() {
	KmsEndpoint *ep;
	KmsSdpSession *session;
	gchar *name;

	name = g_strdup_printf(LOCALNAME);
	session = create_session();

	ep = g_object_new(KMS_TYPE_RTP_ENDPOINT, "localname", name,
						"local-spec", session, NULL);

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