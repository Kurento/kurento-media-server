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

#include "JoinableServiceHandler.h"

#include <log.h>

using ::com::kurento::kms::JoinableServiceHandler;
using ::com::kurento::kms::MediaSessionImpl;
using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::mediaspec::_Direction_VALUES_TO_NAMES;

using ::com::kurento::log::Log;

static Log l("JoinableServiceHandler");
#define i(...) aux_info(l, __VA_ARGS__);
#define d(...) aux_debug(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);

JoinableServiceHandler::JoinableServiceHandler() {
	manager = MediaSessionManager::getInstance();
}

JoinableServiceHandler::~JoinableServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}

void
JoinableServiceHandler::getStreams(std::vector<StreamType::type> &_return,
						const Joinable& joinable) {
	i("getStreams from joinable: %lld", joinable.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(joinable.session);
		JoinableImpl &j = session.getJoinable(joinable);
		j.getStreams(_return);
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::join(const Joinable& from, const Joinable& to,
					const Direction direction) {
	if (_Direction_VALUES_TO_NAMES.find(direction) == _Direction_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid direction");
		throw ex;
	}

	i("join %lld and %lld with direction %s", from.object.id, to.object.id,
			_Direction_VALUES_TO_NAMES.find(direction)->second);
	try {
		if (from.session != to.session) {
			JoinException ex;
			ex.__set_description("Joinables are not in the same "
								"session");
			throw ex;
		}
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		JoinableImpl &t = session.getJoinable(to);
		f.join(t, direction);
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::unjoin(const Joinable& from, const Joinable& to) {
	i("unjoin %lld and %lld", from.object.id, to.object.id);
	try {
		if (from.session != to.session) {
			JoinException ex;
			ex.__set_description("Joinables are not in the same "
			"session");
			throw ex;
		}
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		JoinableImpl &t = session.getJoinable(to);
		f.unjoin(t);
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::joinStream(const Joinable& from, const Joinable& to,
					const StreamType::type stream,
					const Direction direction) {
	if (_Direction_VALUES_TO_NAMES.find(direction) ==
					_Direction_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid direction");
		throw ex;
	}

	if (_StreamType_VALUES_TO_NAMES.find(stream) ==
					_StreamType_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid stream");
		throw ex;
	}

	i("joinStream %s of %lld with %lld with direction %s",
			_StreamType_VALUES_TO_NAMES.find(stream)->second,
			from.object.id, to.object.id,
			_Direction_VALUES_TO_NAMES.find(direction)->second);
	try {
		if (from.session != to.session) {
			JoinException ex;
			ex.__set_description("Joinables are not in the same "
			"session");
			throw ex;
		}
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		JoinableImpl &t = session.getJoinable(to);
		f.join(t, stream, direction);
	} catch(StreamNotFoundException ex) {
		throw ex;
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::unjoinStream(const Joinable& from, const Joinable& to,
						const StreamType::type stream) {
	if (_StreamType_VALUES_TO_NAMES.find(stream) ==
					_StreamType_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid stream");
		throw ex;
	}

	i("unjoinStream %s of %lld with %lld",
			_StreamType_VALUES_TO_NAMES.find(stream)->second,
			from.object.id, to.object.id);
	try {
		if (from.session != to.session) {
			JoinException ex;
			ex.__set_description("Joinables are not in the same "
			"session");
			throw ex;
		}
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		JoinableImpl &t = session.getJoinable(to);
		f.unjoin(t, stream);
	} catch(StreamNotFoundException ex) {
		throw ex;
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::getJoinees(std::vector<Joinable> &_return,
							const Joinable& from) {
	i("getJoinees of %lld", from.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		f.getJoinees(_return);
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::getDirectionJoinees(std::vector<Joinable> &_return,
					const Joinable& from,
					const Direction direction) {
	if (_Direction_VALUES_TO_NAMES.find(direction) ==
					_Direction_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid direction");
		throw ex;
	}

	i("getDirectionJoiness of %lld with direction %s", from.object.id,
			_Direction_VALUES_TO_NAMES.find(direction)->second);
	try {
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		f.getJoinees(_return, direction);
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::getStreamJoinees(std::vector<Joinable> &_return,
						const Joinable& from,
						const StreamType::type stream) {
	if (_StreamType_VALUES_TO_NAMES.find(stream) ==
					_StreamType_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid stream");
		throw ex;
	}

	i("getStreamJoinees of stream %s from %lld",
			_StreamType_VALUES_TO_NAMES.find(stream)->second,
			from.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		f.getJoinees(_return, stream);
	} catch(StreamNotFoundException ex) {
		throw ex;
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::getStreamDirectionJoinees(std::vector<Joinable> &_return,
						const Joinable& from,
						const StreamType::type stream,
						const Direction direction) {
	if (_StreamType_VALUES_TO_NAMES.find(stream) ==
					_StreamType_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid stream");
		throw ex;
	}

	if (_Direction_VALUES_TO_NAMES.find(direction) ==
					_Direction_VALUES_TO_NAMES.end()) {
		JoinException ex;
		ex.__set_description("Invalid direction");
		throw ex;
	}

	i("getStreamDirectionJoinees of stream %s from %lld with direction %s",
			_StreamType_VALUES_TO_NAMES.find(stream)->second,
			from.object.id,
			_Direction_VALUES_TO_NAMES.find(direction)->second);
	try {
		MediaSessionImpl &session = manager->getMediaSession(from.session);
		JoinableImpl &f = session.getJoinable(from);
		f.getJoinees(_return, stream, direction);
	} catch(StreamNotFoundException ex) {
		throw ex;
	} catch(JoinException ex){
		throw ex;
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("MediaSession not found");
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
JoinableServiceHandler::release(const Joinable& joinable) {
	i("release joinable %lld", joinable.object.id);

	try {
		MediaSessionImpl &session = manager->getMediaSession(joinable.session);
		session.deleteJoinable(joinable);
	} catch(JoinableNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("Joinable not found");
		w(e.description);
		throw e;
	} catch (MixerNotFoundException ex) {
		JoinableNotFoundException e;
		e.__set_description("Joinable not found");
		w(e.description);
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
