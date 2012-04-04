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
					const MediaSession& mediaSessionId,
					const std::vector<NetworkConnectionConfig::type>& config) {
	i("CreateNetworkConnection");
}

void
MediaSessionServiceHandler::deleteNetworkConnection(
				const MediaSession& mediaSessionId,
				const NetworkConnection& networConnection) {
	i("deleteNetworkConnection");
}

void
MediaSessionServiceHandler::getNetworkConnections(
					std::vector<NetworkConnection>& _return,
					const MediaSession& mediaSessionId) {
	i("getNetworkConnections");
}

void
MediaSessionServiceHandler::createMixer(Mixer& _return,
				const MediaSession& mediaSessionId,
				const std::vector<MixerConfig::type>& config) {
	i("createMixer");
}

void
MediaSessionServiceHandler::deleteMixer(const MediaSession& mediaSessionId,
							const  Mixer& mixer) {
	i("deleteMixer");
}

void
MediaSessionServiceHandler::getMixers(std::vector<Mixer>& _return,
					const MediaSession& mediaSessionId) {
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