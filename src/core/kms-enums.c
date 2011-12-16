#include <kms-core.h>

GType
kms_media_type_get_type(void) {
	static const GEnumValue values[] = {
		{ KMS_MEDIA_TYPE_AUDIO, "KMS_MEDIA_TYPE_AUDIO", "audio" },
		{ KMS_MEDIA_TYPE_VIDEO, "KMS_MEDIA_TYPE_VIDEO", "video" },
		{ KMS_MEDIA_TYPE_UNKNOWN, "KMS_MEDIA_TYPE_UNKNOWN", "unknown" },
		{ 0, NULL, NULL}
	};

	static GType type = 0;

	if (type == 0) {
		type = g_enum_register_static ("KmsMediaType", values);
	}

	return type;
}

GType
kms_sdp_mode_get_type(void) {
	static const GEnumValue values[] = {
		{ KMS_SDP_MODE_SENDRECV, "KMS_SDP_MODE_SENDRECV", "sendrecv" },
		{ KMS_SDP_MODE_SENDONLY, "KMS_SDP_MODE_SENDONLY", "sendonly" },
		{ KMS_SDP_MODE_RECVONLY, "KMS_SDP_MODE_RECVONLY", "recvonly" },
		{ KMS_SDP_MODE_INACTIVE, "KMS_SDP_MODE_INACTIVE", "inactive" },
		{ 0, NULL, NULL}
	};

	static GType type = 0;

	if (type == 0) {
		type = g_enum_register_static ("KmsSdpMode", values);
	}

	return type;
}
