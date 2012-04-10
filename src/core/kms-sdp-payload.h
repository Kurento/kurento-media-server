#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_SDP_PAYLOAD_H__
#define __KMS_SDP_PAYLOAD_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_SDP_PAYLOAD		(kms_sdp_payload_get_type ())
#define KMS_SDP_PAYLOAD(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_SDP_PAYLOAD, KmsSdpPayload))
#define KMS_IS_SDP_PAYLOAD(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_SDP_PAYLOAD))
#define KMS_SDP_PAYLOAD_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_SDP_PAYLOAD, KmsSdpPayloadClass))
#define KMS_IS_SDP_PAYLOAD_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_SDP_PAYLOAD))
#define KMS_SDP_PAYLOAD_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_SDP_PAYLOAD, KmsSdpPayloadClass))

G_BEGIN_DECLS

typedef struct _KmsSdpPayload		KmsSdpPayload;
typedef struct _KmsSdpPayloadClass	KmsSdpPayloadClass;
typedef struct _KmsSdpPayloadPriv	KmsSdpPayloadPriv;

struct _KmsSdpPayload {
	GObject parent_instance;

	/* instance members */

	KmsSdpPayloadPriv *priv;
};

struct _KmsSdpPayloadClass {
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_SDP_PAYLOAD */
GType kms_sdp_payload_get_type (void);

KmsSdpPayload *kms_sdp_payload_copy(KmsSdpPayload *self);

gchar *kms_sdp_payload_to_string(KmsSdpPayload *self);

gboolean kms_sdp_payload_equals(KmsSdpPayload *a, KmsSdpPayload *b);

GstCaps *kms_sdp_payload_to_caps(KmsSdpPayload *self);

G_END_DECLS

#endif /* __KMS_SDP_PAYLOAD_H__ */
