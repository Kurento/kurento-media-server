#include "types/MediaSessionImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaSessionImpl;
using namespace ::com::kurento::kms::api;

MediaSessionImpl::MediaSessionImpl() : MediaSession() {
	MediaObjectImpl parent;
	__set_parent(parent);
}

void
MediaSessionImpl::createNetworkConnection(NetworkConnection& _return,
		const MediaSession& mediaSession,
		const std::vector<NetworkConnectionConfig::type>& config) {

}

void
MediaSessionImpl::deleteNetworkConnection(const MediaSession& mediaSession,
				const NetworkConnection& networConnection) {
}

void
MediaSessionImpl::getNetworkConnections(
				std::vector<NetworkConnection>& _return,
				const MediaSession& mediaSession) {

}

void
MediaSessionImpl::createMixer(Mixer& _return, const MediaSession& mediaSession,
			const std::vector<MixerConfig::type>& config) {

}

void
MediaSessionImpl::deleteMixer(const MediaSession& mediaSession,
							const Mixer& mixer) {

}

void
MediaSessionImpl::getMixers(std::vector<Mixer>& _return,
					const MediaSession& mediaSession) {

}
