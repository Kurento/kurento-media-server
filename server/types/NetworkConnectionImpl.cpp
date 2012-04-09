#include "types/NetworkConnectionImpl.h"

using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
using ::com::kurento::kms::api::NetworkConnection;

NetworkConnectionImpl::NetworkConnectionImpl() : JoinableImpl(), NetworkConnection() {
	__set_joinable(*this);
}
