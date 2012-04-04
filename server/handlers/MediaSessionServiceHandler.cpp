#include "MediaSessionServiceHandler.h"

using namespace ::com::kurento::kms::api;

namespace com { namespace kurento { namespace kms {

MediaSessionServiceHandler::MediaSessionServiceHandler() {
}

MediaSessionServiceHandler::~MediaSessionServiceHandler() {
}

void
MediaSessionServiceHandler::createNetworkConnection(NetworkConnection& _return,
					const MediaSession& mediaSessionId,
					const std::vector<NetworkConnectionConfig::type>& config) {
}

void
MediaSessionServiceHandler::deleteNetworkConnection(
				const MediaSession& mediaSessionId,
				const NetworkConnection& networConnection) {
}

void
MediaSessionServiceHandler::getNetworkConnections(
					std::vector<NetworkConnection>& _return,
					const MediaSession& mediaSessionId) {
}

void
MediaSessionServiceHandler::createMixer(Mixer& _return,
				const MediaSession& mediaSessionId,
				const std::vector<MixerConfig::type>& config) {
}

void
MediaSessionServiceHandler::deleteMixer(const MediaSession& mediaSessionId,
							const  Mixer& mixer) {
}

void
MediaSessionServiceHandler::getMixers(std::vector<Mixer>& _return,
					const MediaSession& mediaSessionId) {
}

void
MediaSessionServiceHandler::ping(const MediaObject& mediaObject,
							const int32_t timeout) {
}

void
MediaSessionServiceHandler::release(const MediaObject& mediaObject) {
}

}}} // com::kurento::kms