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

#ifndef __KMS_RTP_SENDER_H__
#define __KMS_RTP_SENDER_H__

#include <kms-core.h>
#include <glib-object.h>

#define KMS_TYPE_RTP_SENDER		(kms_rtp_sender_get_type())
#define KMS_RTP_SENDER(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_RTP_SENDER, KmsRtpSender))
#define KMS_IS_RTP_SENDER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_RTP_SENDER))
#define KMS_RTP_SENDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsRtpSenderClass))
#define KMS_IS_RTP_SENDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTP_SENDER))
#define KMS_RTP_SENDER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTP_SENDER, KmsRtpSenderClass))

G_BEGIN_DECLS

typedef struct _KmsRtpSender		KmsRtpSender;
typedef struct _KmsRtpSenderClass	KmsRtpSenderClass;
typedef struct _KmsRtpSenderPriv	KmsRtpSenderPriv;

struct _KmsRtpSender {
	KmsMediaHandlerSink parent_instance;

	/* instance members */

	KmsRtpSenderPriv *priv;
};

struct _KmsRtpSenderClass {
	KmsMediaHandlerSinkClass parent_class;

	/* class members */
};

GType kms_rtp_sender_get_type (void);

G_END_DECLS

#endif /* __KMS_RTP_SENDER_H__ */
