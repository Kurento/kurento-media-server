#include "NetworkConnectionManager.h"
#include <log.h>

using namespace com::kurento::kms;

using ::com::kurento::log::Log;

static Log l("NetworkConnectionImpl");
#define i(...) aux_info(l, __VA_ARGS__);

NetworkConnectionManager::NetworkConnectionManager(){
}

NetworkConnectionManager::~NetworkConnectionManager() {
	std::map<ObjectId, NetworkConnectionImpl *>::iterator it;

	mutex.lock();
	for (it = connections.begin(); it != connections.end(); it++) {
		if (it != connections.end()) {
			delete it->second;
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
	connections[nc->joinable.object.id] = nc;
	mutex.unlock();

	return *nc;
}

void
NetworkConnectionManager::deleteNetworkConnection(const NetworkConnection& nc) {
	std::map<ObjectId, NetworkConnectionImpl *>::iterator it;
	bool found;

	mutex.lock();
	it = connections.find(nc.joinable.object.id);
	if (it != connections.end() && nc == *(it->second)) {
		found = true;
		delete it->second;
		connections.erase(it);
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		NetworkConnectionNotFoundException exception;
		throw exception;
	}
}

JoinableImpl&
NetworkConnectionManager::getJoinable(const Joinable &joinable) {
	std::map<ObjectId, NetworkConnectionImpl *>::iterator it;
	JoinableImpl *j;
	bool found;

	mutex.lock();
	it = connections.find(joinable.object.id);
	if (it != connections.end() && joinable == it->second->joinable) {
		found = true;
		j = it->second;
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		JoinableNotFoundException ex;
		ex.__set_description("Joinable not found");
		throw ex;
	}

	return *j;
}

void
NetworkConnectionManager::getNetworkConnections(
				std::vector<NetworkConnection> &_return) {
	std::map<ObjectId, NetworkConnectionImpl *>::iterator it;

	mutex.lock();
	for (it = connections.begin(); it != connections.end(); it++) {
		_return.push_back(*(it->second));
	}
	mutex.unlock();
}
