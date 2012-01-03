#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_SDP_MEDIA_H__
#define __KMS_SDP_MEDIA_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_SDP_MEDIA		(kms_sdp_media_get_type ())
#define KMS_SDP_MEDIA(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_SDP_MEDIA, KmsSdpMedia))
#define KMS_IS_SDP_MEDIA(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KMS_TYPE_SDP_MEDIA))
#define KMS_SDP_MEDIA_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_SDP_MEDIA, KmsSdpMediaClass))
#define KMS_IS_SDP_MEDIA_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_SDP_MEDIA))
#define KMS_SDP_MEDIA_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_SDP_MEDIA, KmsSdpMediaClass))

typedef struct _KmsSdpMedia		KmsSdpMedia;
typedef struct _KmsSdpMediaClass	KmsSdpMediaClass;
typedef struct _KmsSdpMediaPriv		KmsSdpMediaPriv;

struct _KmsSdpMedia {
	GObject parent_instance;

	/* instance members */

	KmsSdpMediaPriv *priv;
};

struct _KmsSdpMediaClass {
	GObjectClass parent_class;

	/* class members */
};

/* used by KMS_TYPE_SDP_MEDIA */
GType kms_sdp_media_get_type (void);

KmsSdpMedia *kms_sdp_media_copy(KmsSdpMedia *self);
gchar *kms_sdp_media_to_string(KmsSdpMedia *self);

void kms_sdp_media_intersect(KmsSdpMedia *answerer, KmsSdpMedia *offerer,
			       KmsSdpMedia **neg_ans, KmsSdpMedia **neg_off);

#endif /* __KMS_SDP_MEDIA_H__ */
