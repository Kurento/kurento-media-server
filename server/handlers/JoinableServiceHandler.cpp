#include "JoinableServiceHandler.h"

using ::com::kurento::kms::JoinableServiceHandler;

JoinableServiceHandler::JoinableServiceHandler() {
	manager = MediaSessionManager::getInstance();
}

JoinableServiceHandler::~JoinableServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}

void
JoinableServiceHandler::getStreams(std::vector<StreamType::type> &_return,
						const Joinable& joinable) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::join(const Joinable& from, const Joinable& to,
					const Direction::type direction) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::unjoin(const Joinable& from, const Joinable& to) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::joinStream(const Joinable& from, const Joinable& to,
					const StreamType::type stream,
					const Direction::type direction) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::unjoinStream(const Joinable& from, const Joinable& to,
						const StreamType::type stream) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::getJoinees(std::vector<Joinable> &_return,
							const Joinable& from) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::getDirectionJoiness(std::vector<Joinable> &_return,
					const Joinable& from,
					const Direction::type direction) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::getStreamJoinees(std::vector<Joinable> &_return,
						const Joinable& from,
						const StreamType::type stream) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::getStreamDirectionJoiness(std::vector<Joinable> &_return,
						const Joinable& from,
						const StreamType::type stream,
						const Direction::type direction) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableServiceHandler::release(const MediaObject& mediaObject) {
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}
