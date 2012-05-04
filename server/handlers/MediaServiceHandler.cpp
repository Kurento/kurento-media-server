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

#include "MediaServiceHandler.h"
#include "log.h"

using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaServiceHandler");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms {


MediaServerServiceHandler::MediaServerServiceHandler(const ServerConfig &config,
						const SessionSpec &spec):
								config(config),
								spec(spec) {
	manager = MediaSessionManager::getInstance();
}

MediaServerServiceHandler::~MediaServerServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}

void MediaServerServiceHandler::getServerconfig(ServerConfig& _return) {
	_return = config;
}

void MediaServerServiceHandler::createMediaSession(MediaSession& _return) {
	try {
		_return = manager->createMediaSession(spec);
		i("Mediasession created with id %lld", _return.object.id);
	} catch (MediaServerException e) {
		throw e;
	} catch (...) {
		MediaServerException e;
		e.__set_description("Unkown exception found");
		e.__set_code(ErrorCode::type::UNEXPECTED);
		throw e;
	}
}

void MediaServerServiceHandler::deleteMediaSession(const MediaSession& session) {
	i("Deleting media session %lld", session.object.id);
	try {
		manager->deleteMediaSession(session);
	} catch (MediaSessionNotFoundException e) {
		throw e;
	} catch (...) {
		MediaServerException e;
		e.__set_description("Unkown exception found");
		e.__set_code(ErrorCode::type::UNEXPECTED);
		throw e;
	}
}

}}} // com::kurento::kms
