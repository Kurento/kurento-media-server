#ifndef NETWORK_CONNECTION_MANAGER
#define NETWORK_CONNECTION_MANAGER

#include "types/NetworkConnectionImpl.h"
#include "types/MediaSessionImpl.h"
#include <glibmm.h>

using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::ObjectId;
using ::com::kurento::kms::MediaSessionImpl;

namespace com { namespace kurento { namespace kms {

	class NetworkConnectionManager {
	public:
		NetworkConnectionManager(const MediaSessionImpl &session);
		~NetworkConnectionManager();

		NetworkConnectionImpl& createNewtorkConnection();

		NetworkConnectionImpl& getNetworkConnection(const NetworkConnection& nc);

		void deleteMediaSession(const NetworkConnection& nc);

	private:

		const MediaSessionImpl *ms;
		Glib::Mutex mutex; // Protects the list
		std::list<NetworkConnectionImpl *> connections;
	};


}}} // com::kurento::kms::api

#endif /* NETWORK_CONNECTION_MANAGER */
