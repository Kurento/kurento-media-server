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
get_minor(gint32 *ret, gint a, gboolean a_is_set, gint b, gboolean b_is_set) {
	if (a_is_set) {
		if (b_is_set) {
			*ret = a < b ? a : b;
			return TRUE;
		} else {
			*ret = a;
			return a_is_set;
		}
	}

	*ret = b;
	return b_is_set;
}

KmsFraction*
copy_fraction(KmsFraction *orig) {
	KmsFraction *ret = g_object_new(KMS_TYPE_FRACTION, NULL);
	ret->num = orig->num;
	ret->denom = orig->denom;
	return ret;
}

gboolean
get_minor_fraction(KmsFraction **ret, KmsFraction* a, gboolean a_is_set,
					KmsFraction* b, gboolean b_is_set) {
	if (!a_is_set && b_is_set) {
		*ret = copy_fraction(b);
		return TRUE;
	} else if (!b_is_set && a_is_set) {
		*ret = copy_fraction(b);
		return TRUE;
	} else if (a_is_set && b_is_set) {
		*ret = g_object_new(KMS_TYPE_FRACTION, NULL);
		(*ret)->num = a->num < b->num ? a->num : b->num;
		(*ret)->denom = a->denom < b->denom ? a->denom : b->denom;
		return TRUE;
	} else {
		return FALSE;
	}
}

void
kms_payload_rtp_intersect(KmsPayload *answerer, KmsPayload *offerer,
							KmsPayload *neg_answ) {
	GList *keys, *l;

	if (!answerer->__isset_rtp || !offerer->__isset_rtp ||
					answerer->rtp->codecName == NULL ||
					offerer->rtp->codecName == NULL)
		return;

	if (g_ascii_strcasecmp(answerer->rtp->codecName,
						offerer->rtp->codecName) != 0
				|| answerer->rtp->clockRate !=
						offerer->rtp->clockRate) {
		return;
	}

	neg_answ->__isset_rtp = TRUE;
	neg_answ->rtp->id = offerer->rtp->id;
	neg_answ->rtp->clockRate = offerer->rtp->clockRate;
	neg_answ->rtp->codecName = g_strdup(offerer->rtp->codecName);

	neg_answ->rtp->__isset_channels = get_minor((&neg_answ->rtp->channels),
						answerer->rtp->channels,
						answerer->rtp->__isset_channels,
						offerer->rtp->channels,
						offerer->rtp->__isset_channels);

	neg_answ->rtp->__isset_width = get_minor((&neg_answ->rtp->width),
						answerer->rtp->width,
						answerer->rtp->__isset_width,
						offerer->rtp->width,
						offerer->rtp->__isset_width);

	neg_answ->rtp->__isset_height = get_minor((&neg_answ->rtp->height),
						answerer->rtp->height,
						answerer->rtp->__isset_height,
						offerer->rtp->height,
						offerer->rtp->__isset_height);

	neg_answ->rtp->__isset_bitrate = get_minor((&neg_answ->rtp->bitrate),
						answerer->rtp->bitrate,
						answerer->rtp->__isset_bitrate,
						offerer->rtp->bitrate,
						offerer->rtp->__isset_bitrate);

	neg_answ->rtp->__isset_framerate = get_minor_fraction(
					(&neg_answ->rtp->framerate),
					answerer->rtp->framerate,
					answerer->rtp->__isset_framerate,
					offerer->rtp->framerate,
					offerer->rtp->__isset_framerate);

	if (offerer->rtp->__isset_extraParams) {
		neg_answ->rtp->__isset_extraParams = TRUE;
		keys = g_hash_table_get_keys(offerer->rtp->extraParams);
		for (l = keys; l != NULL; l = l->next) {
			gchar *key;
			gchar *value;
			gchar *pvalue = g_hash_table_lookup(
						offerer->rtp->extraParams,
						l->data);

			if (pvalue == NULL)
				continue;

			key = g_strdup(l->data);
			value = g_strdup(pvalue);

			g_hash_table_insert(neg_answ->rtp->extraParams, key,
									value);
		}

		g_list_free(keys);
	}

	if (answerer->rtp->__isset_extraParams) {
		neg_answ->rtp->__isset_extraParams = TRUE;
		keys = g_hash_table_get_keys(answerer->rtp->extraParams);
		for (l = keys; l != NULL; l = l->next) {
			gchar *key;
			gchar *value;
			gchar *pvalue = g_hash_table_lookup(
						answerer->rtp->extraParams,
						l->data);
			gchar *old_value = g_hash_table_lookup(
						neg_answ->rtp->extraParams,
						l->data);

			if (pvalue == NULL || old_value != NULL)
				continue;

			key = g_strdup(l->data);
			value = g_strdup(pvalue);

			g_hash_table_insert(neg_answ->rtp->extraParams, key,
									value);
		}

		g_list_free(keys);
	}
}

static KmsPayload*
kms_payload_clone(KmsPayload *orig) {
	ThriftMemoryBuffer *transport;
	ThriftProtocol *protocol;
	KmsPayload *pay;
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

	pay = g_object_new(KMS_TYPE_PAYLOAD, NULL);
	if (thrift_struct_read(THRIFT_STRUCT(pay), protocol, &err) < 0) {
		if (err != NULL) {
			g_print("Error on write %s\n", err->message);
			g_error_free(err);
		}
		g_object_unref(pay);
		pay = NULL;
		goto end;
	};

end:
	g_object_unref(transport);
	g_object_unref(protocol);
	return pay;
}

gboolean
kms_payload_intersect(KmsPayload *answerer, KmsPayload *offerer,
				KmsPayload **neg_answ, KmsPayload **neg_off) {
	KmsPayload *ret_ans;
	gboolean ret = FALSE;

	if (!KMS_IS_PAYLOAD(answerer) || !KMS_IS_PAYLOAD(offerer) ||
					neg_answ == NULL || neg_off == NULL ||
					*neg_answ != NULL || *neg_off != NULL) {
		g_error("Invalid arguments on kms_media_spec_intersect");
		return FALSE;
	}

	ret_ans = g_object_new(KMS_TYPE_PAYLOAD, NULL);

	kms_payload_rtp_intersect(answerer, offerer, ret_ans);

	ret = ret_ans->__isset_rtp;

	if (!ret) {
		g_object_unref(ret_ans);
	} else {
		*neg_off = kms_payload_clone(ret_ans);
		*neg_answ = ret_ans;
	}

	return ret;
}

GstCaps*
kms_payload_to_caps(KmsPayload *payload, KmsMediaSpec *media) {
	GstCaps *caps = NULL;
	GString *caps_str = NULL;

	if (!KMS_IS_PAYLOAD(payload) || !KMS_IS_MEDIA_SPEC(media))
		goto end;

	if (!payload->__isset_rtp && payload->rtp != NULL)
		goto end;

	caps_str = g_string_new("application/x-rtp");

	if (g_hash_table_lookup(media->type, (gpointer) KMS_MEDIA_TYPE_AUDIO)) {
		g_string_append(caps_str, ",media=(string)audio");
	} else if (g_hash_table_lookup(media->type,
					(gpointer) KMS_MEDIA_TYPE_VIDEO)) {
		g_string_append(caps_str, ",media=(string)video");
	} else {
		g_print("Unsupported media");
		goto end;
	}

	if (payload->rtp->clockRate != 0)
		g_string_append_printf(caps_str, ",clock-rate=(int)%d",
							payload->rtp->clockRate);

	if (payload->rtp->codecName == NULL || payload->rtp->codecName[0] == '\0')
		goto end;

	g_string_append_printf(caps_str, ",encoding-name=(string)%s",
							payload->rtp->codecName);

	g_string_append_printf(caps_str, ",payload=(int)%d",
							payload->rtp->id);

	if (payload->rtp->__isset_bitrate && payload->rtp->bitrate > 0)
		g_string_append_printf(caps_str, ",bandwidth=(int)%d",
							payload->rtp->bitrate);

	/* TODO: Add custom function for each codec, by now just add crc to
	 * amr encoding type.
	 */
	if (g_strcmp0(payload->rtp->codecName, "AMR") == 0) {
		g_string_append(caps_str, ",crc=(string)0");
		g_string_append(caps_str, ",octet-align=(string)1");
		g_string_append(caps_str, ",robust-sorting=(string)0");
		g_string_append(caps_str, ",interleaving=(string)0");
	}

	caps = gst_caps_from_string(caps_str->str);
end:
	if (caps_str != NULL)
		g_string_free(caps_str, TRUE);

	return caps;
}
