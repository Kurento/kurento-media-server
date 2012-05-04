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

#ifndef __KMS_SDP_ENUMS_H__
#define __KMS_SDP_ENUMS_H__

#include <glib-object.h>
#include <kms_media_spec_types.h>

G_BEGIN_DECLS

#define KMS_MEDIA_TYPE (kms_media_type_get_type())

GType kms_media_type_get_type(void) G_GNUC_CONST;

KmsMediaType kms_media_type_from_nick(const gchar *name);

#define KMS_DIRECTION (kms_direction_get_type())

GType kms_direction_get_type(void) G_GNUC_CONST;

G_END_DECLS


#endif /* __KMS_SDP_ENUMS_H__ */
