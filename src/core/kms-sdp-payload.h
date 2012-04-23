#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_SDP_PAYLOAD_H__
#define __KMS_SDP_PAYLOAD_H__

#include <glib-object.h>
#include <gst/gst.h>
#include <kms_media_spec_types.h>
#include <kms_payload_types.h>

G_BEGIN_DECLS

gboolean kms_payload_equals(KmsPayload *a, KmsPayload *b);

gboolean kms_payload_intersect(KmsPayload *answerer, KmsPayload *offerer,
				KmsPayload **neg_answ, KmsPayload **neg_off);

GstCaps *kms_payload_to_caps(KmsPayload *self, KmsMediaSpec *media);

G_END_DECLS

#endif /* __KMS_SDP_PAYLOAD_H__ */
