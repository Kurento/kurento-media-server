#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_PLAYER_H__
#define __KMS_PLAYER_H__

#include <glib-object.h>
#include "kms-enums.h"
#include "kms-sdp-session.h"

/*
 * Type macros.
 */
#define KMS_TYPE_PLAYER			(kms_player_get_type ())
#define KMS_PLAYER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KMS_TYPE_PLAYER, KmsPlayer))
#define KMS_IS_PLAYER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_PLAYER))
#define KMS_PLAYER_GET_INTERFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE((obj), KMS_TYPE_PLAYER, KmsPlayerInterface))

typedef struct _KmsPlayer		KmsPlayer;
typedef struct _KmsPlayerInterface	KmsPlayerInterface;

struct _KmsPlayerInterface {
	KmsResourceInterface parent_class;

	/* class members */

	/*< private >*/
	/* signal callbacks */
	void (*started)(KmsPlayer *player);
	void (*stopped)(KmsPlayer *player);
	void (*failed)(KmsPlayer *player);
	void (*not_found)(KmsPlayer *player);
	void (*bad_uri)(KmsPlayer *player);

	/* Overridable methods */
	void (*set_url)(KmsPlayer *self, gchar *url);
	void (*start)(KmsPlayer *self);
	void (*stop)(KmsPlayer *self);
};

/* used by KMS_TYPE_PLAYER */
GType kms_player_get_type (void);

/*
 * Method definitions.
 */

void kms_player_set_url(KmsPlayer *self, gchar *url);

void kms_player_start(KmsPlayer *self);

void kms_player_stop(KmsPlayer *self);
#endif /* __KMS_PLAYER_H__ */
