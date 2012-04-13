#include "NetworkConnectionServiceHandler.h"

using com::kurento::kms::NetworkConnectionServiceHandler;

NetworkConnectionServiceHandler::NetworkConnectionServiceHandler() {

}

void
NetworkConnectionServiceHandler::generateOffer(SessionSpec &_return,
						const NetworkConnection &nc) {
	// TODO:
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
NetworkConnectionServiceHandler::processAnswer(SessionSpec &_return,
						const NetworkConnection &nc,
						const SessionSpec &anwser) {
	// TODO:
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
NetworkConnectionServiceHandler::processOffer(SessionSpec &_return,
						const NetworkConnection &nc,
						const SessionSpec &offer) {
	// TODO:
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
NetworkConnectionServiceHandler::getLocalDescriptor(SessionSpec &_return,
						const NetworkConnection &nc) {
	// TODO:
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
NetworkConnectionServiceHandler::getRemoteDescriptor(SessionSpec &_return,
						const NetworkConnection &nc) {
	// TODO:
	MediaServerException ex;
	ex.__set_description("Unimplemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}
