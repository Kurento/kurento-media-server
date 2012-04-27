#ifndef __UTILS_H__
#define __UTILS_H__

#include <sessionSpec_types.h>
#include <joinable_types.h>
#include <kms-core.h>

using ::com::kurento::commons::mediaspec::SessionSpec;
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
