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

#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_MEDIA_HANDLER_MANAGER_H__
#define __KMS_MEDIA_HANDLER_MANAGER_H__

#include <glib-object.h>
#include "kms-media-handler-factory.h"

#define KMS_TYPE_MEDIA_HANDLER_MANAGER			(kms_media_handler_manager_get_type())
#define KMS_MEDIA_HANDLER_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_MANAGER, KmsMediaHandlerManager))
#define KMS_IS_MEDIA_HANDLER_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_MANAGER))
#define KMS_MEDIA_HANDLER_MANAGER_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst), KMS_TYPE_MEDIA_HANDLER_MANAGER, KmsMediaHandlerManagerInterface))

G_BEGIN_DECLS

typedef struct _KmsMediaHandlerManager		KmsMediaHandlerManager; /* dummy object */
typedef struct _KmsMediaHandlerManagerInterface	KmsMediaHandlerManagerInterface;

struct _KmsMediaHandlerManagerInterface
{
	GTypeInterface parent_iface;

	/* Virtual methods */
	KmsMediaHandlerFactory *(*get_factory)(KmsMediaHandlerManager *self);
	void (*dispose_factory)(KmsMediaHandlerManager *self,
					KmsMediaHandlerFactory *factory);
};

GType kms_media_handler_manager_get_type(void);

KmsMediaHandlerFactory*
kms_media_handler_manager_get_factory(KmsMediaHandlerManager *self);

void kms_media_handler_manager_dispose_factory(KmsMediaHandlerManager *self,
					KmsMediaHandlerFactory *factory);

G_END_DECLS

#endif /* __KMS_MEDIA_HANDLER_MANAGER_H__ */
