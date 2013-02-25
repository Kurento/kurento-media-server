/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
NetworkConnectionServiceHandler::generateOffer(std::string &_return,
						const NetworkConnection &nc) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(
							nc.joinable.session);
		NetworkConnectionImpl &conn = session.getNetworkConnection(nc);
		conn.generateOffer(_return);
		i("Offer generated for connection %lld", nc.joinable.object.id);
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
NetworkConnectionServiceHandler::processAnswer(std::string &_return,
						const NetworkConnection &nc,
						const std::string &anwser) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(
							nc.joinable.session);
		NetworkConnectionImpl &conn = session.getNetworkConnection(nc);
		conn.processAnswer(_return, anwser);
		i("Answer processed for connection %lld", nc.joinable.object.id);
	} catch(NegotiationException ex) {
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
NetworkConnectionServiceHandler::processOffer(std::string &_return,
						const NetworkConnection &nc,
						const std::string &offer) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(
							nc.joinable.session);
		NetworkConnectionImpl &conn = session.getNetworkConnection(nc);
		conn.processOffer(_return, offer);
		i("Offer processed for connection %lld", nc.joinable.object.id);
	} catch(NegotiationException ex) {
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
NetworkConnectionServiceHandler::getLocalDescriptor(std::string &_return,
						const NetworkConnection &nc) {
	i("Local descriptor requested on connection %lld", nc.joinable.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(
			nc.joinable.session);
		NetworkConnectionImpl &conn = session.getNetworkConnection(nc);
		conn.getLocalDescriptor(_return);
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
NetworkConnectionServiceHandler::getRemoteDescriptor(std::string &_return,
						const NetworkConnection &nc) {
	i("Remote descriptor requested on connection %lld", nc.joinable.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(
			nc.joinable.session);
		NetworkConnectionImpl &conn = session.getNetworkConnection(nc);
		conn.getRemoteDescriptor(_return);
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
