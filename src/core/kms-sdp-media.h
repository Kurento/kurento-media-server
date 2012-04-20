#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_SDP_MEDIA_H__
#define __KMS_SDP_MEDIA_H__

#include <glib-object.h>
#include <kms_media_spec_types.h>

G_BEGIN_DECLS

/* used by KMS_TYPE_SDP_MEDIA */
GType kms_sdp_media_get_type (void);

void kms_media_spec_intersect(KmsMediaSpec *answerer, KmsMediaSpec *offerer,
				KmsMediaSpec **neg_ans, KmsMediaSpec **neg_off);

G_END_DECLS

#endif /* __KMS_SDP_MEDIA_H__ */
