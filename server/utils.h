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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <sessionSpec_types.h>
#include <joinable_types.h>
#include <kms-core.h>

using ::com::kurento::mediaspec::SessionSpec;
using ::com::kurento::kms::api::StreamType;
using ::com::kurento::kms::api::Direction;

namespace com { namespace kurento { namespace kms { namespace utils {

	KmsSessionSpec * convert_session_spec(const SessionSpec &spec);

	void convert_session_spec_to_cpp(SessionSpec &_return,
						const KmsSessionSpec *spec);

	KmsMediaType get_media_type_from_stream(StreamType::type stream) throw (int);

	KmsConnectionMode get_connection_mode_from_direction(Direction direction) throw (int);

	KmsConnectionMode get_inverse_connection_mode(KmsConnectionMode mode) throw (int);

}}}} /* com::kurento::kms::utils */

#endif /* __UTILS_H__ */
