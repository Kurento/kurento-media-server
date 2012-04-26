#include <kms-core.h>
#include <glib.h>
#include <glib-object.h>
#include "memory.h"

#define N_PAYS 5
#define N_MEDIAS 4

#define PAYLOAD(i) (i % 128)
#define CLOCKRATE(i) ((i * 10) % G_MAXINT)

#define PORT(i) ((i * 17) % G_MAXUSHORT)
#define TYPE(i) (i / (KMS_MEDIA_TYPE_VIDEO + 1))
#define MODE(i) (i / (KMS_DIRECTION_INACTIVE + 1))
#define BANDWIDTH(i) ((i * 121) % G_MAXINT)

#define SESSION_ADDR "test.com"
#define SESSION_ID "123456"
#define SESSION_VERSION "654321"

#define TESTS 10000

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

static void
create_payloads(GPtrArray *payloads) {
	gint i;

	for (i = 0; i < N_PAYS; i++) {
		KmsPayload *pay;

		pay = g_object_new(KMS_TYPE_PAYLOAD, NULL);

		if (pay->rtp == NULL) {
			pay->rtp = g_object_new(KMS_TYPE_PAYLOAD_RTP, NULL);
		}

		pay->__isset_rtp = TRUE;

		if (pay->rtp->codecName != NULL)
			g_free(pay->rtp->codecName);
		pay->rtp->codecName = g_value_dup_string(
						g_value_array_get_nth(names, i));
		pay->rtp->id = PAYLOAD(i);
		pay->rtp->clockRate = CLOCKRATE(i);
		pay->rtp->bitrate = BANDWIDTH(i);
		pay->rtp->__isset_bitrate = TRUE;

		g_ptr_array_add(payloads, pay);
	}
}

static void
check_payload(GPtrArray *payloads, gint i) {
	KmsPayload *payload;
	GValue *str;
	gchar *name;
	gint pay, rate;
	gint bandwidth;

	g_assert(i < payloads->len);

	payload = payloads->pdata[i];

	g_assert(payload->__isset_rtp && payload->rtp != NULL);

	name = payload->rtp->codecName;
	pay = payload->rtp->id;
	rate = payload->rtp->clockRate;
	if (payload->rtp->__isset_bitrate)
		bandwidth = payload->rtp->bitrate;
	else
		bandwidth = -2;

	str = g_value_array_get_nth(names, i);

	g_assert(g_ascii_strcasecmp(g_value_get_string(str), name) == 0);
	g_assert(pay == PAYLOAD(i));
	g_assert(rate == CLOCKRATE(i));
	g_assert(bandwidth == BANDWIDTH(i));
}

static void
create_medias(GPtrArray *medias) {
	gint i;
	gint port;
	KmsMediaType type;
	KmsDirection mode;

	for (i = 0; i < N_MEDIAS; i++) {
		KmsMediaSpec *media;

		port = PORT(i);
		type = TYPE(i);
		mode = MODE(i);

		media = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);

		media->transport->__isset_rtp = TRUE;
		media->transport->rtp->port = port;
		media->transport->rtp->address = g_strdup(SESSION_ADDR);

		media->direction = mode;
		g_hash_table_insert(media->type, (gpointer) type, (gpointer) 1);

		create_payloads(media->payloads);

		g_ptr_array_add(medias, media);
	}
}

static void
check_medias(GPtrArray *medias, gint ii) {
	KmsMediaSpec *media;
	gint i;
	gint port;
	gchar *address;
	KmsDirection mode;

	g_assert(ii < medias->len);

	media = medias->pdata[ii];

	g_assert(media->transport->__isset_rtp && media->transport->rtp != NULL);

	port = media->transport->rtp->port;
	address = media->transport->rtp->address;
	mode = media->direction;

	g_assert(media->payloads->len == N_PAYS);
	g_assert(port == PORT(ii));
	g_assert(g_strcmp0(address, SESSION_ADDR) == 0);
	g_assert(mode == MODE(ii));
	g_assert(g_hash_table_lookup(media->type, (gpointer) TYPE(ii)));
	g_assert(g_hash_table_size(media->type) == 1);

	for (i = 0; i < media->payloads->len; i++ ) {
		check_payload(media->payloads,  i);
	}
}

static KmsSessionSpec*
create_session() {
	KmsSessionSpec *session;

	session = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);

	create_medias(session->medias);

	session->id = g_strdup(SESSION_ID);
	session->version = g_strdup(SESSION_VERSION);
	session->__isset_version = TRUE;

	return session;
}

static void
check_session(KmsSessionSpec *session) {
	gchar *id, *version;
	gint i;

	id = session->id;
	g_assert(session->__isset_version);
	version = session->version;

	g_assert(session->medias != NULL);
	g_assert(session->medias->len == N_MEDIAS);

	for (i = 0; i < session->medias->len; i++) {
		check_medias(session->medias, i);
	}

	g_assert(g_strcmp0(id, SESSION_ID) == 0);
	g_assert(g_strcmp0(version, SESSION_VERSION) == 0);
}

gint
main(gint argc, gchar **argv) {
	KmsSessionSpec *session, *copy;
	gint i, new_mem, mem = 0;

	kms_init(&argc, &argv);

	create_names();

	for (i = 0; i < TESTS; i++) {
		session = create_session();
		check_session(session);

		copy = kms_session_spec_copy(session);
		check_session(copy);

		g_object_unref(copy);
		g_object_unref(session);

		new_mem = get_data_memory();
		if (i > 50) {
			if (mem == 0)
				mem = new_mem;
			g_assert(mem == new_mem);
		}
	}

	return 0;
}
