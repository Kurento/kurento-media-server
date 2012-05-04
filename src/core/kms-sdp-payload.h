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

#ifndef __KMS_SDP_PAYLOAD_H__
#define __KMS_SDP_PAYLOAD_H__

#include <glib-object.h>
#include <gst/gst.h>
#include <kms_media_spec_types.h>
#include <kms_payload_types.h>

G_BEGIN_DECLS

gboolean kms_payload_intersect(KmsPayload *answerer, KmsPayload *offerer,
				KmsPayload **neg_answ, KmsPayload **neg_off);

GstCaps *kms_payload_to_caps(KmsPayload *self, KmsMediaSpec *media);

G_END_DECLS

#endif /* __KMS_SDP_PAYLOAD_H__ */
