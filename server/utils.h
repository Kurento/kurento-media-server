#ifndef __UTILS_H__
#define __UTILS_H__

#include <sessionSpec_types.h>
#include <kms-core.h>

using ::com::kurento::commons::mediaspec::SessionSpec;

namespace com { namespace kurento { namespace kms { namespace utils {

	KmsSessionSpec * convert_session_spec(SessionSpec &spec);

}}}} /* com::kurento::kms::utils */

#endif /* __UTILS_H__ */
