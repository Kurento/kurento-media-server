#include "utils.h"
#include <log.h>
#include <common_types.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>

using ::apache::thrift::protocol::TBinaryProtocol;
using ::apache::thrift::transport::TMemoryBuffer;

using ::com::kurento::kms::api::MediaServerException;
using ::com::kurento::kms::api::ErrorCode;

using ::com::kurento::log::Log;

static Log l("utils");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms { namespace utils {

KmsSessionSpec *
convert_session_spec(SessionSpec &spec) {
	boost::shared_ptr<TMemoryBuffer> trans(new TMemoryBuffer());
	TBinaryProtocol proto(trans);
	KmsSessionSpec *local_spec;

	spec.write(&proto);

	guint len;
	guchar *buff;

	trans->getBuffer(&buff, &len);

	local_spec = kms_session_spec_from_binary(buff, len);

	if (local_spec) {
		MediaServerException ex;
		ex.__set_code(ErrorCode::NO_RESOURCES);
		ex.__set_description("Unable to transform session description");
		w(ex.description);
		throw ex;
	}

	return local_spec;
}

}}}} /* com::kurento::kms:utils */
