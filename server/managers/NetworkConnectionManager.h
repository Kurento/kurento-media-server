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

		NetworkConnectionImpl& createNewtorkConnection(const std::vector<NetworkConnectionConfig::type> & config);

		void getNetworkConnections(std::vector<NetworkConnection> &_return);
		NetworkConnectionImpl& getNetworkConnection(const NetworkConnection& nc);

		void deleteMediaSession(const NetworkConnection& nc);

	private:

		Glib::Mutex mutex; // Protects the list
		std::vector<NetworkConnectionImpl *> connections;
	};


}}} // com::kurento::kms::api

#endif /* NETWORK_CONNECTION_MANAGER */
