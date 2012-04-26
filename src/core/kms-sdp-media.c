#include <kms-core.h>
#include <thrift/transport/thrift_memory_buffer.h>
#include <thrift/protocol/thrift_binary_protocol.h>

static gboolean
is_compatible_type(KmsMediaSpec *a, KmsMediaSpec *b) {
	GList *keys, *l;
	gboolean ret = TRUE;

	keys = g_hash_table_get_keys(a->type);
	for (l = keys; l != NULL; l = l->next) {
		if (g_hash_table_lookup(b->type, l->data) == (gpointer) FALSE) {
			ret = FALSE;
			goto end;
		}
	}

end:
	g_list_free(keys);

	return ret;
}

static KmsTransport*
kms_media_transport_copy(KmsTransport *orig) {
	ThriftMemoryBuffer *transport;
	ThriftProtocol *protocol;
	KmsTransport *copy;
	GError *err = NULL;

	transport = g_object_new(THRIFT_TYPE_MEMORY_BUFFER,
				 "buf_size", 2048, NULL);
	protocol = g_object_new(THRIFT_TYPE_BINARY_PROTOCOL,
				"transport", transport, NULL);
	if (thrift_struct_write(THRIFT_STRUCT(orig), protocol, &err) < 0) {
		if (err != NULL) {
			g_print("Error on write %s\n", err->message);
			g_error_free(err);
		}
		goto end;
	}

	copy = g_object_new(KMS_TYPE_TRANSPORT, NULL);
	if (thrift_struct_read(THRIFT_STRUCT(copy), protocol, &err) < 0) {
		if (err != NULL) {
			g_print("Error on read %s\n", err->message);
			g_error_free(err);
		}
		g_object_unref(copy);
		copy = NULL;
		goto end;
	};

end:
	g_object_unref(transport);
	g_object_unref(protocol);
	return copy;
}

gboolean
kms_media_spec_intersect(KmsMediaSpec *answerer, KmsMediaSpec *offerer,
				KmsMediaSpec **neg_ans, KmsMediaSpec **neg_off) {
	KmsDirection a_dir;
	KmsDirection o_dir;
	GPtrArray *ans_payload;
	GPtrArray *off_payload;
	gint i, j;
	GList *keys, *l;

	if (!KMS_IS_MEDIA_SPEC(answerer) || !KMS_IS_MEDIA_SPEC(offerer) ||
					neg_ans == NULL || neg_off == NULL ||
					*neg_ans != NULL || *neg_off != NULL) {
		g_error("Invalid arguments on kms_media_spec_intersect");
		return FALSE;
	}

	if (!is_compatible_type(answerer, offerer)) {
		return FALSE;
	}

	a_dir = answerer->direction;
	o_dir = offerer->direction;

	if (a_dir == KMS_DIRECTION_INACTIVE ||
				o_dir == KMS_DIRECTION_INACTIVE ||
				(a_dir == KMS_DIRECTION_RECVONLY &&
					o_dir == KMS_DIRECTION_RECVONLY) ||
				(a_dir == KMS_DIRECTION_SENDONLY &&
					o_dir == KMS_DIRECTION_SENDONLY)) {
		a_dir = o_dir = KMS_DIRECTION_INACTIVE;
	} else if (a_dir == KMS_DIRECTION_SENDONLY ||
					o_dir == KMS_DIRECTION_RECVONLY) {
		a_dir = KMS_DIRECTION_SENDONLY;
		o_dir = KMS_DIRECTION_RECVONLY;
	} else if (a_dir == KMS_DIRECTION_RECVONLY ||
					o_dir == KMS_DIRECTION_SENDONLY) {
		a_dir = KMS_DIRECTION_RECVONLY;
		o_dir = KMS_DIRECTION_SENDONLY;
	} else {
		a_dir = o_dir = KMS_DIRECTION_SENDRECV;
	}

	*neg_off = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
	*neg_ans = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);

	ans_payload = (*neg_ans)->payloads;
	off_payload = (*neg_off)->payloads;

	for (i = 0; i < answerer->payloads->len; i++) {
		KmsPayload *ans_pay = g_ptr_array_index(answerer->payloads, i);
		for (j = 0; j < offerer->payloads->len; j++) {
			KmsPayload *off_pay = g_ptr_array_index(
							offerer->payloads, j);
			KmsPayload *neg_off = NULL;
			KmsPayload *neg_ans = NULL;
			gboolean neg;

			neg = kms_payload_intersect(ans_pay, off_pay, &neg_ans,
								&neg_off);
			if (neg) {
				g_ptr_array_add(ans_payload, neg_ans);
				g_ptr_array_add(off_payload, neg_off);
				break;
			}
		}
	}

	if ((*neg_off)->transport != NULL) {
		g_object_unref((*neg_off)->transport);
		(*neg_off)->transport = kms_media_transport_copy(
							offerer->transport);
	}

	if ((*neg_ans)->transport != NULL) {
		g_object_unref((*neg_ans)->transport);
		(*neg_ans)->transport = kms_media_transport_copy(
							answerer->transport);
	}

	(*neg_off)->direction = o_dir;
	(*neg_ans)->direction = a_dir;

	keys = g_hash_table_get_keys(offerer->type);

	for (l = keys; l != NULL; l = l->next) {
		g_hash_table_replace((*neg_off)->type,
				     l->data, (gpointer) TRUE);
		g_hash_table_replace((*neg_ans)->type,
				     l->data, (gpointer) TRUE);
	}
	g_list_free(keys);

	return TRUE;
}
