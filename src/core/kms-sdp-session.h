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

#ifndef __KMS_SDP_SESSION_H__
#define __KMS_SDP_SESSION_H__

#include <glib-object.h>
#include <kms_session_spec_types.h>

G_BEGIN_DECLS

KmsSessionSpec *kms_session_spec_copy(KmsSessionSpec *session);

gboolean kms_session_spec_intersect(
			KmsSessionSpec *answerer, KmsSessionSpec *offerer,
			KmsSessionSpec **neg_ans, KmsSessionSpec **neg_off);

KmsSessionSpec *kms_session_spec_from_binary(const guchar data[], guint len);

gint kms_session_spec_to_byte_array(const KmsSessionSpec *spec, guchar data[],
						guint max, GError **error);

G_END_DECLS

#endif /* __KMS_SDP_SESSION_H__ */
