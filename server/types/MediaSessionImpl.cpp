#include "types/MediaSessionImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaSessionImpl;
using namespace ::com::kurento::kms::api;

MediaSessionImpl::MediaSessionImpl() : MediaObjectImpl(), MediaSession() {
	__set_object(*this);
}

NetworkConnection&
MediaSessionImpl::createNetworkConnection(
		const std::vector<NetworkConnectionConfig::type>& config) {
	throw "Not implemented";
}

void
MediaSessionImpl::deleteNetworkConnection(
				const NetworkConnection& networConnection) {
	throw "Not implemented";
}

std::vector<NetworkConnection>&
MediaSessionImpl::getNetworkConnections() {
	throw "Not implemented";
}


Mixer&
MediaSessionImpl::createMixer(const std::vector<MixerConfig::type>& config) {
	throw "Not implemented";
}

void
MediaSessionImpl::deleteMixer(const Mixer& mixer) {
	throw "Not implemented";
}


std::vector<Mixer>&
MediaSessionImpl::getMixers() {
	throw "Not implemented";
}
