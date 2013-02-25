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

#ifndef NETWORK_CONNECTION_MANAGER
#define NETWORK_CONNECTION_MANAGER

#include "types/NetworkConnectionImpl.h"
#include <glibmm.h>

using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::ObjectId;

namespace com { namespace kurento { namespace kms {

	class NetworkConnectionManager {
	public:
		NetworkConnectionManager(const std::string &spec);
		~NetworkConnectionManager();

		NetworkConnectionImpl& createNewtorkConnection(MediaSession &session, const std::vector<NetworkConnectionConfig::type> & config);

		void getNetworkConnections(std::vector<NetworkConnection> &_return);
		NetworkConnectionImpl& getNetworkConnection(const NetworkConnection& nc);
		void deleteNetworkConnection(const NetworkConnection& nc);

		JoinableImpl& getJoinable(const Joinable &joinable);
		void deleteJoinable(const Joinable &joinable);

	private:

		Glib::Mutex mutex; // Protects the list
		std::map<ObjectId, NetworkConnectionImpl *> connections;

		std::string spec;
	};


}}} // com::kurento::kms::api

#endif /* NETWORK_CONNECTION_MANAGER */
