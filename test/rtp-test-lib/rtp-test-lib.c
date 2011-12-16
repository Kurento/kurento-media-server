#include "rtp-test-lib.h"
#include <rtp/kms-rtp-endpoint.h>

#define LOCALNAME "kms/rtp/1"

KmsEndpoint*
create_endpoint() {
	KmsEndpoint *ep;
	gchar *name;

	name = g_strdup_printf(LOCALNAME);

	ep = g_object_new(KMS_TYPE_RTP_ENDPOINT, "localname", name, NULL);

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