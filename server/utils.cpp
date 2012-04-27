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

using ::apache::thrift::TException;

static Log l("utils");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms { namespace utils {

KmsSessionSpec *
convert_session_spec(const SessionSpec &spec) {
	boost::shared_ptr<TMemoryBuffer> trans(new TMemoryBuffer());
	TBinaryProtocol proto(trans);
	KmsSessionSpec *local_spec;

	spec.write(&proto);

	guint len;
	guchar *buff;

	trans->getBuffer(&buff, &len);

	local_spec = kms_session_spec_from_binary(buff, len);

	if (local_spec == NULL) {
		MediaServerException ex;
		ex.__set_code(ErrorCode::NO_RESOURCES);
		ex.__set_description("Unable to transform session description");
		w(ex.description);
		throw ex;
	}

	return local_spec;
}

void
convert_session_spec_to_cpp(SessionSpec &_return, const KmsSessionSpec *spec) {
	boost::shared_ptr<TMemoryBuffer> trans(new TMemoryBuffer());
	TBinaryProtocol proto(trans);
	guchar data[2048];
	gint len;
	GError *err = NULL;

	len = kms_session_spec_to_byte_array(spec, data, 2048, &err);
	if (len < 0) {
		MediaServerException ex;
		ex.__set_code(ErrorCode::NO_RESOURCES);
		if (err != NULL)
			ex.__set_description(err->message);
		else
			ex.__set_description("Unable to transform session description");
		w(ex.description);
		throw ex;
	}

	try {
		trans->write(data, (uint32_t) len);
		_return.read(&proto);
	} catch (TException ex) {
		w(ex.what());
		MediaServerException e;
		e.__set_description(ex.what());
		e.__set_code(ErrorCode::UNEXPECTED);
		throw e;
	}
}

KmsMediaType
get_media_type_from_stream(StreamType::type stream) throw (int) {
	switch (stream) {
	case StreamType::type::AUDIO:
		return KMS_MEDIA_TYPE_AUDIO;
	case StreamType::type::VIDEO:
		return KMS_MEDIA_TYPE_VIDEO;
	default:
		throw -1;
	}
}

KmsConnectionMode
get_connection_mode_from_direction(Direction direction) throw (int) {
	switch (direction) {
	case Direction::INACTIVE:
		return KMS_CONNECTION_MODE_INACTIVE;
	case Direction::RECVONLY:
		return KMS_CONNECTION_MODE_RECVONLY;
	case Direction::SENDONLY:
		return KMS_CONNECTION_MODE_SENDONLY;
	case Direction::SENDRECV:
		return KMS_CONNECTION_MODE_SENDRECV;
	default:
		throw -1;
	}
}

KmsConnectionMode
get_inverse_connection_mode(KmsConnectionMode mode) throw (int) {
	switch (mode) {
	case KMS_CONNECTION_MODE_INACTIVE:
		return KMS_CONNECTION_MODE_INACTIVE;
	case KMS_CONNECTION_MODE_RECVONLY:
		return KMS_CONNECTION_MODE_SENDONLY;
	case KMS_CONNECTION_MODE_SENDONLY:
		return KMS_CONNECTION_MODE_RECVONLY;
	case KMS_CONNECTION_MODE_SENDRECV:
		return KMS_CONNECTION_MODE_SENDRECV;
	default:
		throw -1;
	}
}

}}}} /* com::kurento::kms:utils */
