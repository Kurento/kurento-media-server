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

#include "MediaSessionManager.h"
#include "types/MediaSessionImpl.h"
#include <log.h>

using namespace ::com::kurento::kms;
using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaSessionManager");
#define i(...) aux_info(l, __VA_ARGS__)
#define w(...) aux_warn(l, __VA_ARGS__)

MediaSessionManager::MediaSessionManager() {
}

MediaSessionManager::~MediaSessionManager() {
}

MediaSessionManager *MediaSessionManager::getInstance() {
	static MediaSessionManager *instance = new MediaSessionManager();
	return instance;
}

void MediaSessionManager::releaseInstance(MediaSessionManager* manager) {
	// As instance is a singleton no action is needed
}

MediaSessionImpl &MediaSessionManager::createMediaSession(const std::string &spec) {
	MediaSessionImpl *session = new MediaSessionImpl(spec);
	int size;

	mutex.lock();
	sessions[session->object.id] = session;
	size = sessions.size();
	mutex.unlock();

	i("%d active sessions", size);

	return *session;
}

void MediaSessionManager::deleteMediaSession(const MediaSession &session) {
	std::map<ObjectId, MediaSessionImpl *>::iterator it;
	bool found = FALSE;
	int size;
	MediaSessionImpl *ms = NULL;

	mutex.lock();
	it = sessions.find(session.object.id);
	if (it != sessions.end() && session == *(it->second)) {
		found = true;
		ms = it->second;
		sessions.erase(it);
		size = sessions.size();
	} else {
		found = false;
	}

	mutex.unlock();

	if (!found) {
		MediaSessionNotFoundException exception;
		w("Session %lld not found", session.object.id);
		throw exception;
	} else {
		if (ms != NULL)
			delete ms;
		i("%d active sessions", size);
	}
}



MediaSessionImpl&
MediaSessionManager::getMediaSession(const MediaSession& session) {
	std::map<ObjectId, MediaSessionImpl *>::iterator it;
	MediaSessionImpl *ms;
	bool found = FALSE;

	mutex.lock();
	it = sessions.find(session.object.id);
	if (it != sessions.end() && session== *(it->second)) {
		found = true;
		ms = it->second;
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		throw MediaSessionNotFoundException();
	}

	return *ms;
}
