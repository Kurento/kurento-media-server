#include "types/NetworkConnectionImpl.h"

using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
using ::com::kurento::kms::api::NetworkConnection;

NetworkConnectionImpl::NetworkConnectionImpl(const std::vector<NetworkConnectionConfig::type> & config) :
					JoinableImpl(), NetworkConnection() {
	__set_joinable(*this);

	__set_config(config);
}
