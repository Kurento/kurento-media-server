#include "MediaSessionManager.h"
#include "types/MediaSessionImpl.h"
#include <log.h>

using namespace ::com::kurento::kms;
using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaSessionManager");
#define i(...) aux_info(l, __VA_ARGS__)

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

MediaSessionImpl &MediaSessionManager::createMediaSession() {
	MediaSessionImpl *session = new MediaSessionImpl();
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

	mutex.lock();
	it = sessions.find(session.object.id);
	if (it != sessions.end() && session == *(it->second)) {
		found = true;
		delete it->second;
		sessions.erase(it);
		size = sessions.size();
	} else {
		found = false;
	}

	mutex.unlock();

	if (!found) {
		MediaSessionNotFoundException exception;
		throw exception;
	} else {
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
