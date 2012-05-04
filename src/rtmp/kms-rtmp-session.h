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

#ifndef __KMS_RTMP_SESSION_H__
#define __KMS_RTMP_SESSION_H__

#include <glib-object.h>
#include <kms-core.h>

/*
 * Type macros.
 */
#define KMS_TYPE_RTMP_SESSION		(kms_rtmp_session_get_type ())
#define KMS_RTMP_SESSION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_RTMP_SESSION, KmsRtmpSession))
#define KMS_IS_RTMP_SESSION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_RTMP_SESSION))
#define KMS_RTMP_SESSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_RTMP_SESSION, KmsRtmpSessionClass))
#define KMS_IS_RTMP_SESSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTMP_SESSION))
#define KMS_RTMP_SESSION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTMP_SESSION, KmsRtmpSessionClass))

G_BEGIN_DECLS

typedef struct _KmsRtmpSession		KmsRtmpSession;
typedef struct _KmsRtmpSessionClass	KmsRtmpSessionClass;
typedef struct _KmsRtmpSessionPriv	KmsRtmpSessionPriv;

struct _KmsRtmpSession {
	GObject parent_instance;

	/* instance members */

	KmsRtmpSessionPriv *priv;
};

struct _KmsRtmpSessionClass {
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_RTMP_SESSION */
GType kms_rtmp_session_get_type (void);

KmsRtmpSession *kms_rtmp_session_copy(KmsRtmpSession *self);

gchar *kms_rtmp_session_to_string(KmsRtmpSession *self);

KmsRtmpSession *kms_rtmp_session_create_from_string(const gchar *str);

KmsSessionSpec *kms_rtmp_session_get_sdp_session(KmsRtmpSession *session);

KmsRtmpSession *kms_rtmp_session_create_from_sdp_session(KmsSdpSession *session);

KmsRtmpSession *kms_rtmp_session_intersect(KmsRtmpSession *answerer, KmsRtmpSession *offerer);

gchar *kms_rtmp_session_get_url(KmsRtmpSession *session, gboolean offerer);

G_END_DECLS

#endif /* __KMS_RTMP_SESSION_H__ */
