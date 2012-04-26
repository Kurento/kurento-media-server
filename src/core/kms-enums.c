#include <kms-core.h>

GType
kms_media_type_get_type(void) {
	static const GEnumValue values[] = {
		{ KMS_MEDIA_TYPE_AUDIO, "KMS_MEDIA_TYPE_AUDIO", "audio" },
		{ KMS_MEDIA_TYPE_VIDEO, "KMS_MEDIA_TYPE_VIDEO", "video" },
		{ 0, NULL, NULL}
	};

	static GType type = 0;

	if (type == 0) {
		type = g_enum_register_static ("KmsMediaType", values);
	}

	return type;
}

GType
kms_direction_get_type(void) {
	static const GEnumValue values[] = {
		{ KMS_DIRECTION_SENDRECV, "KMS_DIRECTION_SENDRECV", "sendrecv" },
		{ KMS_DIRECTION_SENDONLY, "KMS_DIRECTION_SENDONLY", "sendonly" },
		{ KMS_DIRECTION_RECVONLY, "KMS_DIRECTION_RECVONLY", "recvonly" },
		{ KMS_DIRECTION_INACTIVE, "KMS_DIRECTION_INACTIVE", "inactive" },
		{ 0, NULL, NULL}
	};

	static GType type = 0;

	if (type == 0) {
		type = g_enum_register_static ("KmsDirection", values);
	}

	return type;
}

KmsMediaType
kms_media_type_from_nick(const gchar* name) {
	GEnumClass *eclass;
	GEnumValue *evalue;

	eclass = G_ENUM_CLASS(g_type_class_peek(KMS_MEDIA_TYPE));
	evalue = g_enum_get_value_by_nick(eclass, name);

	if (evalue == NULL)
		return KMS_MEDIA_TYPE_AUDIO;

	return evalue->value;
}
