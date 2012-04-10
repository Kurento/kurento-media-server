#include "NetworkConnectionManager.h"
#include <log.h>

using namespace com::kurento::kms;

using ::com::kurento::log::Log;

static Log l("NetworkConnectionImpl");
#define i(...) aux_info(l, __VA_ARGS__);

NetworkConnectionManager::NetworkConnectionManager(){
}

NetworkConnectionManager::~NetworkConnectionManager() {
	std::vector<NetworkConnectionImpl *>::iterator it;

	mutex.lock();
	for (it = connections.begin(); it != connections.end(); it++) {
		if (it != connections.end()) {
			delete *it;
		}
	}
	connections.clear();
	mutex.unlock();
}

NetworkConnectionImpl& NetworkConnectionManager::createNewtorkConnection(
		MediaSession &session,
		const std::vector<NetworkConnectionConfig::type> & config) {
	NetworkConnectionImpl *nc = new NetworkConnectionImpl(session, config);

	mutex.lock();
	connections.push_back(nc);
	mutex.unlock();

	return *nc;
}

void
NetworkConnectionManager::deleteNetworkConnection(const NetworkConnection& nc) {
	std::vector<NetworkConnectionImpl *>::iterator it;
	bool found;

	mutex.lock();
	for (it = connections.begin(); it != connections.end(); it++) {
		if (nc == *(*it)) {
			found = true;
			delete *it;
			connections.erase(it);
		}
	}
	mutex.unlock();

	if (!found) {
		NetworkConnectionNotFoundException exception;
		throw exception;
	}
}

void
NetworkConnectionManager::getNetworkConnections(
				std::vector<NetworkConnection> &_return) {
	std::vector<NetworkConnectionImpl *>::iterator it;

	mutex.lock();
	for (it = connections.begin(); it != connections.end(); it++) {
		_return.push_back(**it);
	}
	mutex.unlock();
}
