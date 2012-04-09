#include "NetworkConnectionManager.h"

using namespace com::kurento::kms;


NetworkConnectionManager::NetworkConnectionManager(){
}

NetworkConnectionManager::~NetworkConnectionManager(){
}

NetworkConnectionImpl& NetworkConnectionManager::createNewtorkConnection(const std::vector<NetworkConnectionConfig::type> & config) {
	NetworkConnectionImpl *nc = new NetworkConnectionImpl(config);

	mutex.lock();
	connections.push_back(nc);
	mutex.unlock();

	return *nc;
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
