#include "MediaSessionServiceHandler.h"
#include "log.h"

using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaSessionHandler");
#define i(...) aux_info(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms {

MediaSessionServiceHandler::MediaSessionServiceHandler() {
	manager = MediaSessionManager::getInstance();
}

MediaSessionServiceHandler::~MediaSessionServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}

void
MediaSessionServiceHandler::createNetworkConnection(NetworkConnection& _return,
					const MediaSession& mediaSession,
					const std::vector<NetworkConnectionConfig::type>& config) {
	MediaSessionImpl &session = manager->getMediaSession(mediaSession);
	_return = session.createNetworkConnection(config);

	i("Created NetworkConnection with id: %d", _return.joinable.object.id);
}

void
MediaSessionServiceHandler::deleteNetworkConnection(
				const MediaSession& mediaSession,
				const NetworkConnection& networConnection) {
	i("deleteNetworkConnection");
}

void
MediaSessionServiceHandler::getNetworkConnections(
					std::vector<NetworkConnection>& _return,
					const MediaSession& mediaSession) {
	i("getNetworkConnections");
	MediaSessionImpl &session = manager->getMediaSession(mediaSession);
	session.getNetworkConnections(_return);
}

void
MediaSessionServiceHandler::createMixer(Mixer& _return,
				const MediaSession& mediaSession,
				const std::vector<MixerConfig::type>& config) {
	i("createMixer");
}

void
MediaSessionServiceHandler::deleteMixer(const MediaSession& mediaSession,
							const  Mixer& mixer) {
	i("deleteMixer");
}

void
MediaSessionServiceHandler::getMixers(std::vector<Mixer>& _return,
					const MediaSession& mediaSession) {
	i("getMixers");
}

void
MediaSessionServiceHandler::ping(const MediaObject& mediaObject,
							const int32_t timeout) {
	i("ping");
}

void
MediaSessionServiceHandler::release(const MediaObject& mediaObject) {
	i("release");
}

}}} // com::kurento::kms