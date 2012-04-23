#include <kms-core.h>

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

void
kms_payload_rtp_intersect(KmsPayload *answerer, KmsPayload *offerer,
			  KmsPayload *neg_answ, KmsPayload *neg_off) {
	if (!answerer->__isset_rtp || !offerer->__isset_rtp ||
					answerer->rtp->codecName != NULL ||
					offerer->rtp->codecName != NULL)
		return;

	if (g_ascii_strcasecmp(answerer->rtp->codecName,
						offerer->rtp->codecName) != 0
				|| answerer->rtp->clockRate !=
						offerer->rtp->clockRate) {
		return;
	}

	neg_answ->__isset_rtp = TRUE;
	neg_off->__isset_rtp = TRUE;
	neg_answ->rtp->id = offerer->rtp->id;
	neg_off->rtp->id = offerer->rtp->id;
	neg_answ->rtp->clockRate = offerer->rtp->clockRate;
	neg_off->rtp->clockRate = offerer->rtp->clockRate;
	neg_answ->rtp->codecName = g_strdup(offerer->rtp->codecName);
	neg_off->rtp->codecName = g_strdup(offerer->rtp->codecName);

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
}

gboolean
kms_payload_intersect(KmsPayload *answerer, KmsPayload *offerer,
				KmsPayload **neg_answ, KmsPayload **neg_off) {
	KmsPayload *ret_ans, *ret_off;
	gboolean ret = FALSE;

	ret_ans = g_object_new(KMS_TYPE_PAYLOAD, NULL);
	ret_off = g_object_new(KMS_TYPE_PAYLOAD, NULL);

	kms_payload_rtp_intersect(answerer, offerer, ret_ans, ret_off);

	ret = ret_ans->__isset_rtp && ret_off->__isset_rtp;

	if (!ret) {
		g_object_unref(ret_ans);
		g_object_unref(ret_off);
	} else {
		*neg_answ = ret_ans;
		*neg_off = ret_off;
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

	if (g_hash_table_lookup(media->type, (gpointer) KMS_MEDIA_TYPE_AUDIO)
								!= NULL) {
		g_string_append(caps_str, ",media=(string)audio");
	} else if (g_hash_table_lookup(media->type,
				(gpointer) KMS_MEDIA_TYPE_VIDEO) != NULL) {
		g_string_append(caps_str, ",media=(string)video");
	} else {
		g_print("Unsupported media");
		goto end;
	}

	if (payload->rtp-> clockRate != 0)
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
