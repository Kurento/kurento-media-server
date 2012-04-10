#include "types/MediaSessionImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaSessionImpl;
using ::com::kurento::kms::NetworkConnectionManager;
using namespace ::com::kurento::kms::api;

MediaSessionImpl::MediaSessionImpl() : MediaObjectImpl(), MediaSession() {
	__set_object(*this);

	// TODO: Generate token and set it

	ncManager = new NetworkConnectionManager();
	// TODO: Subscribe to ping controller
}

MediaSessionImpl::~MediaSessionImpl() throw() {
	delete ncManager;
}

NetworkConnection&
MediaSessionImpl::createNetworkConnection(
		const std::vector<NetworkConnectionConfig::type>& config) {
	NetworkConnection &nc = ncManager->createNewtorkConnection(*this, config);
	return nc;
}

void
MediaSessionImpl::deleteNetworkConnection(
				const NetworkConnection& networkConnection) {
	ncManager->deleteNetworkConnection(networkConnection);
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

void
MediaSessionImpl::ping(const int32_t timeout) {
	// TODO: Implement this method
	throw "Not implemented";
}
