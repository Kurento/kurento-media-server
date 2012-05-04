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

#ifndef __KMS_MEDIA_HANDLER_FACTORY_H__
#define __KMS_MEDIA_HANDLER_FACTORY_H__

#include <glib-object.h>
#include "kms-media-handler-sink.h"
#include "kms-media-handler-src.h"

#define KMS_TYPE_MEDIA_HANDLER_FACTORY			(kms_media_handler_factory_get_type())
#define KMS_MEDIA_HANDLER_FACTORY(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_MEDIA_HANDLER_FACTORY, KmsMediaHandlerFactory))
#define KMS_IS_MEDIA_HANDLER_FACTORY(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_MEDIA_HANDLER_FACTORY))
#define KMS_MEDIA_HANDLER_FACTORY_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst), KMS_TYPE_MEDIA_HANDLER_FACTORY, KmsMediaHandlerFactoryInterface))

G_BEGIN_DECLS

typedef struct _KmsMediaHandlerFactory		KmsMediaHandlerFactory; /* dummy object */
typedef struct _KmsMediaHandlerFactoryInterface	KmsMediaHandlerFactoryInterface;

struct _KmsMediaHandlerFactoryInterface {
	GTypeInterface parent_iface;

	/* Virtual methods */
	KmsMediaHandlerSrc * (*get_src)(KmsMediaHandlerFactory *self);
	KmsMediaHandlerSink * (*get_sink)(KmsMediaHandlerFactory *self);
};

GType kms_media_handler_factory_get_type (void);

/* Interface methods */

KmsMediaHandlerSrc *kms_media_handler_factory_get_src(KmsMediaHandlerFactory *self);
KmsMediaHandlerSink *kms_media_handler_factory_get_sink(KmsMediaHandlerFactory *self);

G_END_DECLS

#endif /* __KMS_MEDIA_HANDLER_FACTORY_H__ */
