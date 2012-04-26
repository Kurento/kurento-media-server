#include "types/JoinableImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaObjectImpl;
using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::Joinable;
using ::com::kurento::kms::api::MediaServerException;
using ::com::kurento::kms::api::ErrorCode;

JoinableImpl::JoinableImpl(MediaSession &session) :
					MediaObjectImpl(session.object.token),
					Joinable() {
	__set_object(*this);
	__set_session(session);
}

void
JoinableImpl::getStreams(std::vector<StreamType::type> &_return) {
	/* TODO: FIXME: By now all joinables has AUDIO and VIDEO by default.
	 * In the future this has to be get from endpoints in kmsclib */
	_return.push_back(StreamType::AUDIO);
	_return.push_back(StreamType::VIDEO);
}

void
JoinableImpl::join(const JoinableImpl& to, const Direction direction) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::join(const JoinableImpl& to, const StreamType::type stream,
						const Direction direction) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::unjoin(const JoinableImpl& to) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::unjoin(const JoinableImpl& to, const StreamType::type stream) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return,
						const Direction direction) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return,
						const StreamType::type stream) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return,
						const StreamType::type stream,
						const Direction direction) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}
