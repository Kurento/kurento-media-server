#ifndef __KMS_MEDIA_HANDLER_FACTORY_H__
#define __KMS_MEDIA_HANDLER_FACTORY_H__

#include <glib-object.h>

#define KMS_TYPE_MEDIA_HANDLER_FACTORY			(kms_media_handler_factory_get_type())
#define KMS_MEDIA_HANDLER_FACTORY(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_FACTORY, KmsMediaHandlerFactory))
#define KMS_IS_MEDIA_HANDLER_FACTORY(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_FACTORY))
#define KMS_MEDIA_HANDLER_FACTORY_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst), KMS_TYPE_MEDIA_HANDLER_FACTORY, KmsMediaHandlerFactoryInterface))

typedef struct _KmsMediaHandlerFactory		KmsMediaHandlerFactory; /* dummy object */
typedef struct _KmsMediaHandlerFactoryInterface	KmsMediaHandlerFactoryInterface;

struct _KmsMediaHandlerFactoryInterface
{
	GTypeInterface parent_iface;

	/* Virtual methods */
	/* TODO: Add methods */
};

GType kms_media_handler_factory_get_type (void);

#endif /* __KMS_MEDIA_HANDLER_FACTORY_H__ */
