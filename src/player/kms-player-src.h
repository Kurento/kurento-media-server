#ifndef __KMS_PLAYER_SRC_H__
#define __KMS_PLAYER_SRC_H__

#include <glib-object.h>
#include <kms-core.h>

#define KMS_TYPE_PLAYER_SRC		(kms_player_src_get_type())
#define KMS_PLAYER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_PLAYER_SRC, KmsPlayerSrc))
#define KMS_IS_PLAYER_SRC(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_PLAYER_SRC))
#define KMS_PLAYER_SRC_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsPlayerSrcClass))
#define KMS_IS_PLAYER_SRC_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_PLAYER_SRC))
#define KMS_PLAYER_SRC_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_PLAYER_SRC, KmsPlayerSrcClass))

G_BEGIN_DECLS

typedef struct _KmsPlayerSrc		KmsPlayerSrc;
typedef struct _KmsPlayerSrcClass	KmsPlayerSrcClass;
typedef struct _KmsPlayerSrcPriv	KmsPlayerSrcPriv;

struct _KmsPlayerSrc {
	KmsMediaHandlerSrc parent_instance;

	/* instance members */

	KmsPlayerSrcPriv *priv;
};

struct _KmsPlayerSrcClass {
	KmsMediaHandlerSrcClass parent_class;

	/* class members */
};

GType kms_player_src_get_type (void);

G_END_DECLS

#endif /* __KMS_PLAYER_SRC_H__ */
