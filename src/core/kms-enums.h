#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_SDP_ENUMS_H__
#define __KMS_SDP_ENUMS_H__

#include <glib-object.h>
#include <kms_media_spec_types.h>

G_BEGIN_DECLS

#define KMS_MEDIA_TYPE (kms_media_type_get_type())

GType kms_media_type_get_type(void) G_GNUC_CONST;

KmsMediaType kms_media_type_from_nick(const gchar *name);

#define KMS_SDP_MODE (kms_sdp_mode_get_type())

GType kms_sdp_mode_get_type(void) G_GNUC_CONST;

typedef enum _KmsSdpMode {
	KMS_SDP_MODE_SENDRECV,
	KMS_SDP_MODE_SENDONLY,
	KMS_SDP_MODE_RECVONLY,
	KMS_SDP_MODE_INACTIVE
} KmsSdpMode;

G_END_DECLS


#endif /* __KMS_SDP_ENUMS_H__ */
