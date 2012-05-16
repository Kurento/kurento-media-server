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

#include <kms-core.h>
#include <thrift/transport/thrift_memory_buffer.h>
#include <thrift/protocol/thrift_binary_protocol.h>

static gboolean
is_compatible_transport(KmsMediaSpec *a, KmsMediaSpec *b) {
	return (a->transport->__isset_rtmp && b->transport->__isset_rtmp) ||
					(a->transport->__isset_rtp &&
						b->transport->__isset_rtp);
}

static gboolean
is_compatible_type(KmsMediaSpec *a, KmsMediaSpec *b) {
	GList *keys, *l;
	gboolean ret = TRUE;

	keys = g_hash_table_get_keys(a->type);
	for (l = keys; l != NULL; l = l->next) {
		if (!g_hash_table_lookup(b->type, l->data)) {
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

static void
intersect_transport(KmsTransport *answerer, KmsTransport *offerer,
				KmsTransport *neg_ans, KmsTransport *neg_off) {
	if (!KMS_IS_TRANSPORT(answerer) || !KMS_IS_TRANSPORT(offerer) ||
						!KMS_IS_TRANSPORT(neg_ans) ||
						!KMS_IS_TRANSPORT(neg_off))
		return;

	if (!answerer->__isset_rtmp || !offerer->__isset_rtmp ||
						!neg_ans->__isset_rtmp ||
						!neg_off->__isset_rtmp)
		return;

	if (offerer->rtmp->__isset_publish) {
		g_free(neg_ans->rtmp->play);
		neg_ans->rtmp->__isset_play = TRUE;
		neg_ans->rtmp->play = g_strdup(offerer->rtmp->publish);
	} else {
		goto fail;
	}

	if (answerer->rtmp->__isset_publish) {
		g_free(neg_off->rtmp->play);
		neg_off->rtmp->__isset_play = TRUE;
		neg_off->rtmp->play = g_strdup(answerer->rtmp->publish);
	} else {
		goto fail;
	}

	if (offerer->rtmp->__isset_url) {
		g_free(neg_off->rtmp->url);
		neg_off->rtmp->__isset_url = TRUE;
		neg_off->rtmp->url = g_strdup(offerer->rtmp->url);
		g_free(neg_ans->rtmp->url);
		neg_ans->rtmp->__isset_url = TRUE;
		neg_ans->rtmp->url = g_strdup(offerer->rtmp->url);
	} else if (answerer->rtmp->__isset_url) {
		g_free(neg_off->rtmp->url);
		neg_off->rtmp->__isset_url = TRUE;
		neg_off->rtmp->url = g_strdup(answerer->rtmp->url);
		g_free(neg_ans->rtmp->url);
		neg_ans->rtmp->__isset_url = TRUE;
		neg_ans->rtmp->url = g_strdup(answerer->rtmp->url);
	} else {
		goto fail;
	}

	return;
fail:
	neg_ans->__isset_rtmp = FALSE;
	neg_off->__isset_rtmp = FALSE;
}

gboolean
kms_media_spec_intersect(KmsMediaSpec *answerer, KmsMediaSpec *offerer,
				KmsMediaSpec **neg_ans, KmsMediaSpec **neg_off) {
	KmsDirection a_dir;
	KmsDirection o_dir;
	GPtrArray *ans_payload;
	GPtrArray *off_payload;
	KmsMediaSpec *ret_ans, *ret_off;
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

	if (!is_compatible_transport(answerer, offerer)) {
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

	ret_off = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
	ret_ans = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);

	ans_payload = ret_ans->payloads;
	off_payload = ret_off->payloads;

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
	// TODO: if no payloads are set, set to inactive?

	if (offerer->transport != NULL) {
		g_object_unref(ret_off->transport);
		ret_off->transport = kms_media_transport_copy(
							offerer->transport);
	}

	if (answerer->transport != NULL) {
		g_object_unref(ret_ans->transport);
		ret_ans->transport = kms_media_transport_copy(
							answerer->transport);
	}

	intersect_transport(answerer->transport, offerer->transport,
							ret_ans->transport,
							ret_off->transport);

	if ((!ret_ans->transport->__isset_rtp &&
					!ret_ans->transport->__isset_rtmp) ||
				(!ret_off->transport->__isset_rtp &&
					!ret_off->transport->__isset_rtmp))
		goto fail;

	ret_off->direction = o_dir;
	ret_ans->direction = a_dir;

	keys = g_hash_table_get_keys(offerer->type);

	for (l = keys; l != NULL; l = l->next) {
		g_hash_table_replace(ret_off->type,
				     l->data, (gpointer) TRUE);
		g_hash_table_replace(ret_ans->type,
				     l->data, (gpointer) TRUE);
	}
	g_list_free(keys);

	*neg_ans = ret_ans;
	*neg_off = ret_off;
	return TRUE;

fail:
	g_object_unref(ret_ans);
	g_object_unref(ret_off);

	return FALSE;
}
