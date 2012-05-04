/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined (__KMS_RTP_H_INSIDE__)
#error "Only <rtp/kms-rtp.h> can be included directly."
#endif

#ifndef __KMS_RTP_ENDPOINT_H__
#define __KMS_RTP_ENDPOINT_H__

#include <glib-object.h>
#include <kms-core.h>

/*
 * Type macros.
 */
#define KMS_TYPE_RTP_ENDPOINT			(kms_rtp_endpoint_get_type ())
#define KMS_RTP_ENDPOINT(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_RTP_ENDPOINT, KmsRtpEndpoint))
#define KMS_IS_RTP_ENDPOINT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_RTP_ENDPOINT))
#define KMS_RTP_ENDPOINT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_RTP_ENDPOINT, KmsRtpEndpointClass))
#define KMS_IS_RTP_ENDPOINT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTP_ENDPOINT))
#define KMS_RTP_ENDPOINT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTP_ENDPOINT, KmsRtpEndpointClass))

#define KMS_RTP_ENDPOINT_ERROR (g_quark_from_string("KmsRtpEndpointError"))

G_BEGIN_DECLS

typedef struct _KmsRtpEndpoint		KmsRtpEndpoint;
typedef struct _KmsRtpEndpointClass	KmsRtpEndpointClass;
typedef struct _KmsRtpEndpointPriv	KmsRtpEndpointPriv;

struct _KmsRtpEndpoint {
	KmsEndpoint parent_instance;

	/* instance members */

	KmsRtpEndpointPriv *priv;
};

struct _KmsRtpEndpointClass {
	KmsEndpointClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_RTP_ENDPOINT */
GType kms_rtp_endpoint_get_type (void);

/*
 * Method definitions.
 */

G_END_DECLS

#endif /* __KMS_RTP_ENDPOINT_H__ */
