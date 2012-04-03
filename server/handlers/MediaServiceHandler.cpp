#include "MediaServiceHandler.h"
#include "log.h"

using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaServiceHandler");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms {


MediaServerServiceHandler::MediaServerServiceHandler()
{
	manager = MediaSessionManager::getInstance();
}

MediaServerServiceHandler::~MediaServerServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}


void MediaServerServiceHandler::createMediaSession(MediaSession& _return) {
	i("createMediaSession");
	_return = manager->createMediaSession();
}

void MediaServerServiceHandler::deleteMediaSession(const MediaSession& session) {
	i("deleteMediaSession");
	manager->deleteMediaSession(session);
}

}}} // com::kurento::kms
