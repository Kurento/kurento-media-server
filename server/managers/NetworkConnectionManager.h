#ifndef NETWORK_CONNECTION_MANAGER
#define NETWORK_CONNECTION_MANAGER

#include "types/NetworkConnectionImpl.h"
#include <glibmm.h>

using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::ObjectId;

namespace com { namespace kurento { namespace kms {

	class NetworkConnectionManager {
	public:
		NetworkConnectionManager();
		~NetworkConnectionManager();

		NetworkConnectionImpl& createNewtorkConnection(MediaSession &session, const std::vector<NetworkConnectionConfig::type> & config);

		void getNetworkConnections(std::vector<NetworkConnection> &_return);
		NetworkConnectionImpl& getNetworkConnection(const NetworkConnection& nc);
		void deleteNetworkConnection(const NetworkConnection& nc);

		JoinableImpl& getJoinable(const Joinable &joinable);

	private:

		Glib::Mutex mutex; // Protects the list
		std::map<ObjectId, NetworkConnectionImpl *> connections;
	};


}}} // com::kurento::kms::api

#endif /* NETWORK_CONNECTION_MANAGER */
