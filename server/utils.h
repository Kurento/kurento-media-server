#ifndef __UTILS_H__
#define __UTILS_H__

#include <sessionSpec_types.h>
#include <kms-core.h>

using ::com::kurento::commons::mediaspec::SessionSpec;

namespace com { namespace kurento { namespace kms { namespace utils {

	KmsSessionSpec * convert_session_spec(const SessionSpec &spec);

	void convert_session_spec_to_cpp(SessionSpec &_return,
						const KmsSessionSpec *spec);
}}}} /* com::kurento::kms::utils */

#endif /* __UTILS_H__ */
