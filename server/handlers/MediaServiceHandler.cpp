#include "MediaServiceHandler.h"

using namespace ::com::kurento::kms::api;

namespace com { namespace kurento { namespace kms {


MediaServerServiceHandler::MediaServerServiceHandler()
{
	manager = MediaSessionManager::getInstance();
}

MediaServerServiceHandler::~MediaServerServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}


void MediaServerServiceHandler::createMediaSession(MediaSession& _return) {
	_return = manager->createMediaSession();
}

void MediaServerServiceHandler::deleteMediaSession(const MediaSession& session) {
	manager->deleteMediaSession(session);
}

}}} // com::kurento::kms
