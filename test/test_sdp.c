#include <kms-core.h>
#include <glib.h>
#include <glib-object.h>
#include <mcheck.h>

#define N_PAYS 10
#define N_MEDIAS 4

#define PAYLOAD(i) (i % 128)
#define CLOCKRATE(i) ((i * 10) % G_MAXINT)

#define PORT(i) ((i * 17) % G_MAXUSHORT)
#define TYPE(i) (i / (KMS_MEDIA_TYPE_VIDEO + 1))
#define MODE(i) (i / (KMS_SDP_MODE_INACTIVE + 1))
#define BANDWIDTH(i) ((i * 121) % G_MAXLONG)

#define SESSION_ADDR "kurento.com"

static GValueArray *names = NULL;

static void
create_names() {
	gint i;

	if (names != NULL)
		return;

	names = g_value_array_new(N_PAYS);

	for (i = 0; i < N_PAYS; i++) {
		gchar *name = g_strdup_printf("name%d", i);
		GValue vname = G_VALUE_INIT;

		g_value_init(&vname, G_TYPE_STRING);
		g_value_take_string(&vname, name);
		g_value_array_append(names, &vname);
		g_value_unset(&vname);
	}
}

static GValueArray*
create_payloads() {
	GValueArray *payloads;
	gint i;

	payloads = g_value_array_new(N_PAYS);

	for (i = 0; i < N_PAYS; i++) {
		KmsSdpPayload *pay;
		GValue vpay = G_VALUE_INIT;

		pay = g_object_new(KMS_TYPE_SDP_PAYLOAD, "name",
				   g_value_get_string(
					   g_value_array_get_nth(
						   names, i)),
		     "payload", PAYLOAD(i),
				   "clockrate", CLOCKRATE(i),
				   NULL);

		g_value_init(&vpay, KMS_TYPE_SDP_PAYLOAD);
		g_value_take_object(&vpay, pay);
		g_value_array_append(payloads, &vpay);
		g_value_unset(&vpay);
	}

	return payloads;
}

static void
check_payload(GValueArray *payloads, KmsSdpMedia *media, gint i) {
	GValue *val;
	GValue *str;
	gchar *name;
	gint pay, rate;
	KmsSdpMedia *media_aux;

	val = g_value_array_get_nth(payloads, i);
	g_object_get(g_value_get_object(val), "name", &name,
		     "payload", &pay,
	      "clockrate", &rate,
	      "media", &media_aux,
	      NULL);

	str = g_value_array_get_nth(names, i);

	/*
	 g _print("name: >%s<\n", na*me);
	 g_print("name:->%s<\n", g_value_get_string(str));
	 */
	g_assert(g_strcmp0(g_value_get_string(str), name) == 0);
	g_assert(pay == PAYLOAD(i));
	g_assert(rate == CLOCKRATE(i));
	g_assert(media_aux == media);

	g_free(name);
	if (media_aux != NULL)
		g_object_unref(media_aux);
}

static GValueArray*
create_medias() {
	GValueArray *payloads;
	GValueArray *medias;
	gint i;
	gint port;
	KmsMediaType type;
	KmsSdpMode mode;
	glong bandwidth;

	medias = g_value_array_new(N_MEDIAS);

	for (i = 0; i < N_MEDIAS; i++) {
		KmsSdpMedia *media;
		GValue vmedia = G_VALUE_INIT;

		payloads = create_payloads();

		port = PORT(i);
		type = TYPE(i);
		mode = MODE(i);
		bandwidth = BANDWIDTH(i);

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

static void
check_medias(GValueArray *medias, gint ii) {
	GValueArray *payloads;
	GValue *val;
	KmsSdpMedia *media;
	gint i;
	gint port;
	KmsMediaType type;
	KmsSdpMode mode;
	glong bandwidth;

	val = g_value_array_get_nth(medias, ii);
	media = g_value_get_object(val);

	g_object_get(media, "port", &port, "bandwidth", &bandwidth,
				"mode", &mode, "type", &type,
				"payloads", &payloads, NULL);

	g_assert(payloads != NULL);
	g_assert(port == PORT(ii));
	g_assert(bandwidth == BANDWIDTH(ii));
	g_assert(mode == MODE(ii));
	g_assert(type == TYPE(ii));

	for (i = 0; i < payloads->n_values; i++ ) {
		check_payload(payloads, media, i);
	}

	g_value_array_free(payloads);
}

static KmsSdpSession*
create_session() {
	KmsSdpSession *session;
	GValueArray *medias;

	medias = create_medias();

	session = g_object_new(KMS_TYPE_SDP_SESSION, "medias", medias,
						"address", SESSION_ADDR, NULL);

	g_value_array_free(medias);

	return session;
}

static void
check_session(KmsSdpSession *session) {
	GValueArray *medias;
	gchar *addr;
	gint i;

	g_object_get(session, "medias", &medias, "address", &addr, NULL);

	g_assert(medias != NULL);

	for (i = 0; i < medias->n_values; i++) {
		check_medias(medias, i);
	}

	g_assert(g_strcmp0(addr, SESSION_ADDR) == 0);

	g_value_array_free(medias);
	g_free(addr);
}

gint
main(gint argc, gchar **argv) {
	KmsSdpSession *session;

	kms_init(&argc, &argv);

	create_names();

	/*
	do {
	*/

	session = create_session();

	check_session(session);

	g_object_unref(session);

	/*
	}while(g_print("loop\n"), sleep(1), 1);
	*/

	return 0;
}