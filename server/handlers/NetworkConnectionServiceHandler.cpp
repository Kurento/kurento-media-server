#include "NetworkConnectionServiceHandler.h"

#include <log.h>

using com::kurento::kms::NetworkConnectionServiceHandler;
using ::com::kurento::kms::MediaSessionImpl;
using ::com::kurento::kms::NetworkConnectionImpl;

using ::com::kurento::log::Log;

static Log l("NetworkConnectionService");
#define i(...) aux_info(l, __VA_ARGS__);
#define d(...) aux_debug(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);

NetworkConnectionServiceHandler::NetworkConnectionServiceHandler() {

}

void
NetworkConnectionServiceHandler::generateOffer(SessionSpec &_return,
						const NetworkConnection &nc) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(nc.joinable.session);
		NetworkConnectionImpl &conn = session.getNetworkConnection(nc);
		_return = conn.generateOffer();
		i("Offer generated for session %lld", nc.joinable.object.id);
	} catch(StreamNotFoundException ex) {
		throw ex;
	} catch(JoinException ex){
		throw ex;
	} catch(NetworkConnectionNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		NetworkConnectionNotFoundException e;
		throw e;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
NetworkConnectionServiceHandler::processAnswer(SessionSpec &_return,
						const NetworkConnection &nc,
						const SessionSpec &anwser) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(nc.joinable.session);
		NetworkConnectionImpl &conn = session.getNetworkConnection(nc);
		_return = conn.processAnswer(anwser);
		i("Answer processed for session %lld", nc.joinable.object.id);
	} catch(StreamNotFoundException ex) {
		throw ex;
	} catch(JoinException ex){
		throw ex;
	} catch(NetworkConnectionNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		NetworkConnectionNotFoundException e;
		throw e;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
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
