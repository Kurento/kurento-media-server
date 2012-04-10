#ifndef NETWORK_CONNECTION_IMPL
#define NETWORK_CONNECTION_IMPL

#include "networkConnection_types.h"
#include "types/JoinableImpl.h"

using namespace ::com::kurento::kms::api;
using namespace ::com::kurento::commons::mediaspec;

namespace com { namespace kurento { namespace kms {

	class NetworkConnectionImpl : public NetworkConnection,
					public virtual JoinableImpl {
	public:
		NetworkConnectionImpl(MediaSession &session, const std::vector<NetworkConnectionConfig::type> & config);
		~NetworkConnectionImpl() throw() {};

		SessionSpec& generateOffer();
		SessionSpec& processAnswer(const SessionSpec& anwser);
		SessionSpec& processOffer(const SessionSpec& offer);
		SessionSpec& getLocalDescriptor();
		SessionSpec& getRemoteDescriptor();
	};

}}} // com::kurento::kms

#endif /* NETWORK_CONNECTION_IMPL */