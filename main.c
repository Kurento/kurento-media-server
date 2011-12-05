#include "endpoint.h"
#include <glib.h>

int main(int argc, char **argv) {
	GObject *ep;
	GValue val = G_VALUE_INIT;
	GError *err = NULL;
	KmsConnection *conn = NULL;

	g_type_init();

	ep = g_object_new(KMS_TYPE_ENDPOINT, "localname", "test_ep", NULL);

	if (ep == NULL) {
		g_print("Create endpont is: NULL\n");
		return 1;
	}

	g_value_init(&val, G_TYPE_STRING);

	g_object_get_property(ep, "localname", &val);

	g_print("Created ep with localname: %s\n", g_value_get_string(&val));

	conn = kms_endpoint_create_connection(KMS_ENDPOINT(ep),
						KMS_CONNECTION_TYPE_RTP, &err);

	if (conn == NULL) {
		g_print("Connection can not be created: %s\n", err->message);;
		g_error_free(err);
		goto end;
	}

end:
	g_object_unref(ep);

	return 0;
}
