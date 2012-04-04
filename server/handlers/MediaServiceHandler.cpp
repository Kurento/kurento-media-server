#include "MediaServiceHandler.h"
#include "log.h"

using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaServiceHandler");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms {


MediaServerServiceHandler::MediaServerServiceHandler(ServerConfig *config)
{
	manager = MediaSessionManager::getInstance();
	this->config = config;
}

MediaServerServiceHandler::~MediaServerServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}

void MediaServerServiceHandler::getServerconfig(ServerConfig& _return) {
	_return = *config;
}

void MediaServerServiceHandler::createMediaSession(MediaSession& _return) {
	_return = manager->createMediaSession();
	i("Mediasession created with id %d", _return.parent.id);
}

void MediaServerServiceHandler::deleteMediaSession(const MediaSession& session) {
	i("Deleting media session %d", session.parent.id);
	manager->deleteMediaSession(session);
}

}}} // com::kurento::kms
