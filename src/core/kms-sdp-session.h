#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_SDP_SESSION_H__
#define __KMS_SDP_SESSION_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_SDP_SESSION		(kms_sdp_session_get_type ())
#define KMS_SDP_SESSION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_SDP_SESSION, KmsSdpSession))
#define KMS_IS_SDP_SESSION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_SDP_SESSION))
#define KMS_SDP_SESSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_SDP_SESSION, KmsSdpSessionClass))
#define KMS_IS_SDP_SESSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_SDP_SESSION))
#define KMS_SDP_SESSION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_SDP_SESSION, KmsSdpSessionClass))

typedef struct _KmsSdpSession		KmsSdpSession;
typedef struct _KmsSdpSessionClass	KmsSdpSessionClass;
typedef struct _KmsSdpSessionPriv	KmsSdpSessionPriv;

struct _KmsSdpSession {
	GObject parent_instance;

	/* instance members */

	KmsSdpSessionPriv *priv;
};

struct _KmsSdpSessionClass {
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_SDP_SESSION */
GType kms_sdp_session_get_type (void);

KmsSdpSession *kms_sdp_session_copy(KmsSdpSession *session);

#endif /* __KMS_SDP_SESSION_H__ */
