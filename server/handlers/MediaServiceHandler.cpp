#include "MediaServiceHandler.h"
#include "log.h"

using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaServiceHandler");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

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
	try {
		_return = manager->createMediaSession();
		i("Mediasession created with id %d", _return.object.id);
	} catch (MediaServerException e) {
		throw e;
	} catch (...) {
		MediaServerException e;
		e.__set_description("Unkown exception found");
		e.__set_code(ErrorCode::type::UNEXPECTED);
		throw e;
	}
}

void MediaServerServiceHandler::deleteMediaSession(const MediaSession& session) {
	i("Deleting media session %d", session.object.id);
	try {
		manager->deleteMediaSession(session);
	} catch (MediaSessionNotFoundException e) {
		throw e;
	} catch (...) {
		w("Unotified exeption while delting media session");
	}
}

}}} // com::kurento::kms
