/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "NetworkConnectionManager.h"
#include <log.h>

using namespace com::kurento::kms;

using ::com::kurento::log::Log;

static Log l("NetworkConnectionImpl");
#define i(...) aux_info(l, __VA_ARGS__);

NetworkConnectionManager::NetworkConnectionManager(const SessionSpec &spec):
								spec(spec) {
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
	NetworkConnectionImpl *nc = new NetworkConnectionImpl(session, config,
									spec);

	mutex.lock();
	connections[nc->joinable.object.id] = nc;
	mutex.unlock();

	return *nc;
}

void
NetworkConnectionManager::deleteNetworkConnection(const NetworkConnection& nc) {
	std::map<ObjectId, NetworkConnectionImpl *>::iterator it;
	bool found;
	int size = -1;

	mutex.lock();
	it = connections.find(nc.joinable.object.id);
	if (it != connections.end() && nc == *(it->second)) {
		found = true;
		delete it->second;
		connections.erase(it);
		size = connections.size();
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		NetworkConnectionNotFoundException exception;
		throw exception;
	}

	i("%d active connections for session %lld", size,
						nc.joinable.session.object.id);
}

void
NetworkConnectionManager::deleteJoinable(const Joinable &joinable) {
	std::map<ObjectId, NetworkConnectionImpl *>::iterator it;
	bool found;
	int size = -1;

	mutex.lock();
	it = connections.find(joinable.object.id);
	if (it != connections.end() && joinable == it->second->joinable) {
		found = true;
		delete it->second;
		connections.erase(it);
		size = connections.size();
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		JoinableNotFoundException exception;
		throw exception;
	}

	i("%d active connections for session %lld", size,
						joinable.session.object.id);
}


NetworkConnectionImpl&
NetworkConnectionManager::getNetworkConnection(const NetworkConnection &nc) {
	std::map<ObjectId, NetworkConnectionImpl *>::iterator it;
	NetworkConnectionImpl *conn;
	bool found;

	mutex.lock();
	it = connections.find(nc.joinable.object.id);
	if (it != connections.end() && nc == *(it->second)) {
		found = true;
		conn = it->second;
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		JoinableNotFoundException ex;
		ex.__set_description("Joinable not found");
		throw ex;
	}

	return *conn;
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
