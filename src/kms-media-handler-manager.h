#ifndef __KMS_MEDIA_HANDLER_MANAGER_H__
#define __KMS_MEDIA_HANDLER_MANAGER_H__

#include <glib-object.h>

#define KMS_TYPE_MEDIA_HANDLER_MANAGER			(kms_media_handler_manager_get_type())
#define KMS_MEDIA_HANDLER_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_MANAGER, KmsMediaHandlerManager))
#define KMS_IS_MEDIA_HANDLER_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_MANAGER))
#define KMS_MEDIA_HANDLER_MANAGER_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst), KMS_TYPE_MEDIA_HANDLER_MANAGER, KmsMediaHandlerManagerInterface))

typedef struct _KmsMediaHandlerManager		KmsMediaHandlerManager; /* dummy object */
typedef struct _KmsMediaHandlerManagerInterface	KmsMediaHandlerManagerInterface;

struct _KmsMediaHandlerManagerInterface
{
	GTypeInterface parent_iface;

	/* Virtual methods */
	void (*get_factory)(KmsMediaHandlerManager *self);
};

GType kms_media_handler_manager_get_type(void);

/* TODO: make this function return a KmsMediaHandlerFactory */
void kms_media_handler_manager_get_factory(KmsMediaHandlerManager *self);

#endif /* __KMS_MEDIA_HANDLER_MANAGER_H__ */
