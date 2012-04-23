#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_SDP_SESSION_H__
#define __KMS_SDP_SESSION_H__

#include <glib-object.h>
#include <kms_session_spec_types.h>

G_BEGIN_DECLS

KmsSessionSpec *kms_session_spec_copy(KmsSessionSpec *session);

gboolean kms_session_spec_intersect(
			KmsSessionSpec *answerer, KmsSessionSpec *offerer,
			KmsSessionSpec **neg_ans, KmsSessionSpec **neg_off);

G_END_DECLS

#endif /* __KMS_SDP_SESSION_H__ */
