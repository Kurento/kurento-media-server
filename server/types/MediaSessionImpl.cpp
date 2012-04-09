#include "types/MediaSessionImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaSessionImpl;
using ::com::kurento::kms::NetworkConnectionManager;
using namespace ::com::kurento::kms::api;

MediaSessionImpl::MediaSessionImpl() : MediaObjectImpl(), MediaSession() {
	__set_object(*this);

	ncManager = new NetworkConnectionManager();
}

MediaSessionImpl::~MediaSessionImpl() throw() {
	delete ncManager;
}

NetworkConnection&
MediaSessionImpl::createNetworkConnection(
		const std::vector<NetworkConnectionConfig::type>& config) {
	NetworkConnection &nc = ncManager->createNewtorkConnection(config);
	nc.__set_session(*this);
	return nc;
}

void
MediaSessionImpl::deleteNetworkConnection(
				const NetworkConnection& networConnection) {
	ncManager->deleteNetworkConnection(networConnection);
}

void
MediaSessionImpl::getNetworkConnections(std::vector<NetworkConnection> &_return) {
	ncManager->getNetworkConnections(_return);
}


Mixer&
MediaSessionImpl::createMixer(const std::vector<MixerConfig::type>& config) {
	throw "Not implemented";
}

void
MediaSessionImpl::deleteMixer(const Mixer& mixer) {
	throw "Not implemented";
}


void
MediaSessionImpl::getMixers(std::vector<Mixer> &_return) {
	throw "Not implemented";
}
