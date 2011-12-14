#ifndef __KMS_RTP_CONNECTION_H__
#define __KMS_RTP_CONNECTION_H__

#include <glib-object.h>
#include "kms-connection.h"

/*
 * Type macros.
 */
#define KMS_TYPE_RTP_CONNECTION		(kms_rtp_connection_get_type ())
#define KMS_RTP_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_RTP_CONNECTION, KmsRtpConnection))
#define KMS_IS_RTP_CONNECTION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_RTP_CONNECTION))
#define KMS_RTP_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_RTP_CONNECTION, KmsRtpConnectionClass))
#define KMS_IS_RTP_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTP_CONNECTION))
#define KMS_RTP_CONNECTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTP_CONNECTION, KmsRtpConnectionClass))

typedef struct _KmsRtpConnection		KmsRtpConnection;
typedef struct _KmsRtpConnectionClass		KmsRtpConnectionClass;
typedef struct _KmsRtpConnectionPriv		KmsRtpConnectionPriv;

struct _KmsRtpConnection {
	KmsConnection parent_instance;

	/* instance members */

	/*
	KmsRtpConnectionPriv *priv;
	*/
};

struct _KmsRtpConnectionClass {
	KmsConnectionClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_RTP_CONNECTION */
GType kms_rtp_connection_get_type (void);

/*
 * Method definitions.
 */

#endif /* __KMS_RTP_CONNECTION_H__ */
