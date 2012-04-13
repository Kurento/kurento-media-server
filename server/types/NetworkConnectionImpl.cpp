#include "types/NetworkConnectionImpl.h"

using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
using ::com::kurento::kms::api::NetworkConnection;

NetworkConnectionImpl::NetworkConnectionImpl(MediaSession &session, const std::vector<NetworkConnectionConfig::type> & config) :
				JoinableImpl(session), NetworkConnection() {
	__set_joinable(*this);

	__set_config(config);
}

SessionSpec&
NetworkConnectionImpl::generateOffer() {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

SessionSpec&
NetworkConnectionImpl::processAnswer(const SessionSpec& anwser) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

SessionSpec&
NetworkConnectionImpl::processOffer(const SessionSpec& offer) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

SessionSpec&
NetworkConnectionImpl::getLocalDescriptor() {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}
