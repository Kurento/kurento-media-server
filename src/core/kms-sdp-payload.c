#include <kms-core.h>

static gboolean
kms_payload_rtp_equals(KmsPayloadRtp *a, KmsPayloadRtp *b) {
	//TODO: Implement this function
	return FALSE;
}

gboolean
kms_payload_equals(KmsPayload *a, KmsPayload *b) {

	if (a->__isset_rtp && b->__isset_rtp)
		return kms_payload_rtp_equals(a->rtp, b->rtp);

	return FALSE;
}

gboolean
kms_payload_intersect(KmsPayload *answerer, KmsPayload *offerer,
				KmsPayload **neg_answ, KmsPayload **neg_off) {
	// TODO: Implement this function
	return FALSE;
}

GstCaps*
kms_payload_to_caps(KmsPayload *payload, KmsMediaSpec *media) {
	GstCaps *caps = NULL;
	GString *caps_str = NULL;
// 	KmsMediaType type;

	if (!KMS_IS_PAYLOAD(payload) || !KMS_IS_MEDIA_SPEC(media))
		goto end;

	if (!payload->__isset_rtp && payload->rtp != NULL)
		goto end;

	caps_str = g_string_new("application/x-rtp");

	// TODO: Set Media type correctly
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
