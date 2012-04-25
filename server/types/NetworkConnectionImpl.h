#ifndef NETWORK_CONNECTION_IMPL
#define NETWORK_CONNECTION_IMPL

#include "networkConnection_types.h"
#include "types/JoinableImpl.h"

using namespace ::com::kurento::kms::api;
using ::com::kurento::commons::mediaspec::SessionSpec;

namespace com { namespace kurento { namespace kms {

	class NetworkConnectionImpl : public NetworkConnection,
					public virtual JoinableImpl {
	public:
		NetworkConnectionImpl(MediaSession &session,
				const std::vector<NetworkConnectionConfig::type> & config,
				const SessionSpec &spec);
		~NetworkConnectionImpl() throw();

		void generateOffer(SessionSpec& _return);
		void processAnswer(SessionSpec& _return, const SessionSpec& anwser);
		void processOffer(SessionSpec& _return, const SessionSpec& offer);
		void getLocalDescriptor(SessionSpec& _return);
		void getRemoteDescriptor(SessionSpec& _return);

	private:
		SessionSpec spec;
		KmsConnection *rtp_connection = NULL;
	};

}}} // com::kurento::kms

#endif /* NETWORK_CONNECTION_IMPL */
