#include "kms-sdp-payload.h"
#include "kms-sdp-session.h"
#include <glib.h>
#include <glib-object.h>
#include <mcheck.h>

#define N_PAYS 10000
#define PAYLOAD(i) (i % 128)
#define CLOCKRATE(i) ((i * 10) % G_MAXUSHORT)

gint
main(gint argc, gchar **argv) {
	KmsSdpPayload *pay;
	GValueArray *payloads;
	GValueArray *names;
	int i;

	g_type_init();

	payloads = g_value_array_new(0);
	names = g_value_array_new(0);

	for (i = 0; i < N_PAYS; i++) {
		GValue vpay = G_VALUE_INIT;
		GValue vname = G_VALUE_INIT;
		gchar *name = g_strdup_printf("name%d", i);

		g_value_init(&vname, G_TYPE_STRING);
		g_value_take_string(&vname, name);
		g_value_array_append(names, &vname);

		pay = g_object_new(KMS_TYPE_SDP_PAYLOAD, "name", name,
					"payload", PAYLOAD(i),
					"clockrate", CLOCKRATE(i),
					NULL);

		g_value_init(&vpay, KMS_TYPE_SDP_PAYLOAD);
		g_value_take_object(&vpay, pay);
		g_value_array_append(payloads, &vpay);

		g_value_unset(&vname);
		g_value_unset(&vpay);
	}

	for (i = 0; i < payloads->n_values; i++ ) {
		GValue *val;
		GValue *str;
		gchar *name;
		gint pay, rate;

		val = g_value_array_get_nth(payloads, i);
		g_object_get(g_value_get_object(val), "name", &name,
						"payload", &pay,
						"clockrate", &rate,
						NULL);

		str = g_value_array_get_nth(names, i);

		/*
		g_print("name: >%s<\n", name);
		g_print("name:->%s<\n", g_value_get_string(str));
		*/
		g_assert(g_strcmp0(g_value_get_string(str), name) == 0);
		g_assert(pay == PAYLOAD(i));
		g_assert(rate == CLOCKRATE(i));

		g_free(name);
	}

	g_value_array_free(payloads);
	g_value_array_free(names);

	return 0;
}