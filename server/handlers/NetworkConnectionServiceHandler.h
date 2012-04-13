#ifndef NETWORK_CONNECTION_SERVICE_HANDLER
#define NETWORK_CONNECTION_SERVICE_HANDLER

#include "NetworkConnectionService.h"
#include "JoinableServiceHandler.h"

using ::com::kurento::kms::api::NetworkConnectionServiceIf;
using ::com::kurento::kms::api::MediaObject;
using ::com::kurento::commons::mediaspec::SessionSpec;
using ::com::kurento::kms::api::NetworkConnection;

namespace com { namespace kurento { namespace kms {

class NetworkConnectionServiceHandler :
				virtual public JoinableServiceHandler,
				virtual public NetworkConnectionServiceIf {
public:
	NetworkConnectionServiceHandler();
	~NetworkConnectionServiceHandler() {};

	void generateOffer(SessionSpec &_return, const NetworkConnection &nc);
	void processAnswer(SessionSpec &_return, const NetworkConnection &nc, const SessionSpec &anwser);
	void processOffer(SessionSpec &_return, const NetworkConnection &nc, const SessionSpec &offer);
	void getLocalDescriptor(SessionSpec &_return, const NetworkConnection &nc);
	void getRemoteDescriptor(SessionSpec &_return, const NetworkConnection &nc);
};

}}} // com::kurento::kms::api

#endif /* NETWORK_CONNECTION_SERVICE_HANDLER */
