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
