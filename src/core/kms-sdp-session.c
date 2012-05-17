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

gboolean
kms_session_spec_intersect(KmsSessionSpec *answerer, KmsSessionSpec *offerer,
						KmsSessionSpec **neg_answ,
						KmsSessionSpec **neg_off) {
	GPtrArray *off_list, *answ_list, *new_off_list, *new_answ_list;
	GPtrArray *used_medias;
	gint i, j;

	if (!KMS_IS_SESSION_SPEC(answerer) || !KMS_IS_SESSION_SPEC(offerer) ||
					neg_answ == NULL || neg_off == NULL ||
					*neg_answ != NULL || *neg_off != NULL) {
		g_error("Invalid arguments on kms_session_spec_intersect");
		return FALSE;
	}

	*neg_off = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);
	*neg_answ = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);

	off_list = offerer->medias;
	answ_list = answerer->medias;

	new_off_list = (*neg_off)->medias;
	new_answ_list = (*neg_answ)->medias;

	used_medias = g_ptr_array_new();

	for (i = 0; i < off_list->len; i++) {
		KmsMediaSpec *of_media = g_ptr_array_index(off_list, i);
		KmsMediaSpec *neg_of_media = NULL;
		KmsMediaSpec *neg_ans_media = NULL;
		gboolean neg = FALSE;

		for (j = 0; j < answ_list->len; j++) {
			KmsMediaSpec *a_media = g_ptr_array_index(answ_list, j);

			if (kms_g_ptr_array_contains(used_medias, a_media))
				continue;

			neg = kms_media_spec_intersect(a_media, of_media,
						 &neg_ans_media, &neg_of_media);
			if (neg) {
				g_ptr_array_add(used_medias, a_media);
				break;
			}
		}

		if (!neg) {
			GList *l;
			GList *keys = g_hash_table_get_keys(of_media->type);

			neg_of_media = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
			neg_of_media->direction = KMS_DIRECTION_INACTIVE;

			neg_ans_media = g_object_new(KMS_TYPE_MEDIA_SPEC, NULL);
			neg_ans_media->direction = KMS_DIRECTION_INACTIVE;

			for (l = keys; l != NULL; l = l->next) {
				g_hash_table_replace(neg_ans_media->type,
						l->data, (gpointer) TRUE);
				g_hash_table_replace(neg_of_media->type,
						     l->data, (gpointer) TRUE);
			}
			g_list_free(keys);
		}

		g_ptr_array_add(new_off_list, neg_of_media);
		g_ptr_array_add(new_answ_list, neg_ans_media);
	}

	g_ptr_array_free(used_medias, TRUE);

	if (offerer->id != NULL) {
		(*neg_off)->id = g_strdup(offerer->id);
		(*neg_answ)->id = g_strdup(offerer->id);
	}

	if (offerer->__isset_version && offerer->version != NULL) {
		(*neg_off)->version = g_strdup(offerer->version);
		(*neg_off)->__isset_version = TRUE;
		(*neg_answ)->version = g_strdup(answerer->version);
		(*neg_answ)->__isset_version = TRUE;
	}

	return TRUE;
}

gint
kms_session_spec_to_byte_array(const KmsSessionSpec *spec, guchar data[],
						guint max, GError **err) {
	ThriftTransport *transport;
	ThriftProtocol *protocol;
	gint len = -1;

	transport = g_object_new(THRIFT_TYPE_MEMORY_BUFFER,
						"buf_size", max, NULL);
	protocol = g_object_new(THRIFT_TYPE_BINARY_PROTOCOL,
						"transport", transport, NULL);
	len = thrift_struct_write(THRIFT_STRUCT(spec), protocol, err);
	if (len < 0)
		goto end;

	len = thrift_transport_read(transport, data, max, err);

end:
	g_object_unref(transport);
	g_object_unref(protocol);
	return len;
}

KmsSessionSpec*
kms_session_spec_copy(KmsSessionSpec *self) {
	ThriftMemoryBuffer *transport;
	ThriftProtocol *protocol;
	KmsSessionSpec *spec;
	GError *err = NULL;

	transport = g_object_new(THRIFT_TYPE_MEMORY_BUFFER,
							"buf_size", 2048, NULL);
	protocol = g_object_new(THRIFT_TYPE_BINARY_PROTOCOL,
						"transport", transport, NULL);
	if (thrift_struct_write(THRIFT_STRUCT(self), protocol, &err) < 0) {
		if (err != NULL) {
			g_print("Error on write %s\n", err->message);
			g_error_free(err);
		}
		goto end;
	}

	spec = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);
	if (thrift_struct_read(THRIFT_STRUCT(spec), protocol, &err) < 0) {
		if (err != NULL) {
			g_print("Error on write %s\n", err->message);
			g_error_free(err);
		}
		g_object_unref(spec);
		spec = NULL;
		goto end;
	};
end:
	g_object_unref(transport);
	g_object_unref(protocol);
	return spec;
}

KmsSessionSpec*
kms_session_spec_from_binary(const guchar data[], guint len) {
	ThriftMemoryBuffer *transport;
	ThriftProtocol *protocol;
	KmsSessionSpec *spec;
	GError *error = NULL;

	transport = g_object_new(THRIFT_TYPE_MEMORY_BUFFER, "buf_size", len,
									NULL);
	protocol = g_object_new(THRIFT_TYPE_BINARY_PROTOCOL,
						"transport", transport, NULL);

	if (thrift_transport_write(THRIFT_TRANSPORT(transport), (gpointer) data,
							len, &error) < 0) {
		if (error != NULL) {
			g_error("Error on write %s\n", error->message);
			g_error_free(error);
		}
		goto end;
	}

	spec = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);
	if (thrift_struct_read(THRIFT_STRUCT(spec), protocol, &error) < 0) {
		if (error != NULL) {
			g_error("Error on write %s\n", error->message);
			g_error_free(error);
		}
		g_object_unref(spec);
		spec = NULL;
		goto end;
	};

end:
	g_object_unref(transport);
	g_object_unref(protocol);
	return spec;
}

KmsSessionSpec *
kms_session_spec_filter_transport(KmsSessionSpec *session, GType type) {
	KmsSessionSpec *ret;
	GSList *l, *remove = NULL;
	gint i;

	if (session == NULL)
		return NULL;

	ret = kms_session_spec_copy(session);

	if (ret == NULL)
		return NULL;

	for (i = 0; i < ret->medias->len; i++) {
		KmsMediaSpec *spec;

		spec = ret->medias->pdata[i];
		if (type == KMS_TYPE_TRANSPORT_RTMP) {
			if (!spec->transport->__isset_rtmp) {
				remove = g_slist_prepend(remove, spec);
			} else {
				spec->transport->__isset_rtp = FALSE;
			}
		} else if (type == KMS_TYPE_TRANSPORT_RTP) {
			if (!spec->transport->__isset_rtp) {
				remove = g_slist_prepend(remove, spec);
			} else {
				spec->transport->__isset_rtmp = FALSE;
			}
		}
	}

	for (l = remove; l != NULL; l = l->next) {
		g_ptr_array_remove(ret->medias, l->data);
	}

	// ptr_array unrefs the data
	g_slist_free(remove);

	return ret;
}
