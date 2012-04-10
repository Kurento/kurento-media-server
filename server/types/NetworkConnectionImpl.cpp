#include "types/NetworkConnectionImpl.h"

using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
using ::com::kurento::kms::api::NetworkConnection;

NetworkConnectionImpl::NetworkConnectionImpl(MediaSession &session, const std::vector<NetworkConnectionConfig::type> & config) :
				JoinableImpl(session), NetworkConnection() {
	__set_joinable(*this);

	__set_config(config);
}
