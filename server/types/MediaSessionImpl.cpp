#include "types/MediaSessionImpl.h"

#include <glibmm.h>
#include <uuid/uuid.h>

using ::com::kurento::kms::MediaSessionImpl;
using ::com::kurento::kms::NetworkConnectionManager;
using namespace ::com::kurento::kms::api;

MediaSessionImpl::MediaSessionImpl() : MediaObjectImpl(), MediaSession() {
	uuid_t uuid;
	char *uuid_str;

	__set_object(*this);

	uuid_str = (char *) g_malloc(sizeof(char) * 37);
	uuid_generate(uuid);
	uuid_unparse(uuid, uuid_str);
	std::string tk;
	tk.append(uuid_str);
	g_free(uuid_str);
	object.__set_token(tk);

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
