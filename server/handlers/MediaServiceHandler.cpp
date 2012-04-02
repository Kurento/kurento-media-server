#include "MediaServiceHandler.h"

using namespace ::com::kurento::kms::api;

namespace com { namespace kurento { namespace kms {


MediaServerServiceHandler::MediaServerServiceHandler()
{
	std::cout << "Initializing" << std::endl;
}

void MediaServerServiceHandler::createMediaSession(MediaSession& _return) {
	// Your implementation goes here
	printf("createMediaSession\n");

	MediaServerException exception;
	exception.__set_description("Not implemented");
	exception.__set_code(ErrorCode::NO_RESOURCES);
	throw exception;
}

void MediaServerServiceHandler::deleteMediaSession(const MediaSession& session) {
	// Your implementation goes here
	printf("deleteMediaSession\n");

	MediaServerException exception;
	exception.__set_description("Not implemented");
	exception.__set_code(ErrorCode::NO_RESOURCES);
	throw exception;
}

}}} // com::kurento::kms
