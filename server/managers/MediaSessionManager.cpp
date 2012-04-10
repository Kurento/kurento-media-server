#include "MediaSessionManager.h"
#include "types/MediaSessionImpl.h"

using namespace ::com::kurento::kms;
using namespace ::com::kurento::kms::api;

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

	mutex.lock();
	sessions.push_back(session);
	mutex.unlock();

	return *session;
}

void MediaSessionManager::deleteMediaSession(const MediaSession &session) {
	deleteMediaSession(session.object);
}

void MediaSessionManager::deleteMediaSession(const MediaObject &object) {
	std::list<MediaSessionImpl *>::iterator it;
	bool found = FALSE;

	mutex.lock();
	for (it = sessions.begin(); !found && (it != sessions.end()); it++) {
		if (object == (*it)->object) {
			found = true;
			delete *it;
			sessions.erase(it);
		}
	}
	mutex.unlock();

	if (!found) {
		MediaSessionNotFoundException exception;
		throw exception;
	}
}



MediaSessionImpl&
MediaSessionManager::getMediaSession(const MediaSession& session) {
	return getMediaSession(session.object);
}

MediaSessionImpl&
MediaSessionManager::getMediaSession(const MediaObject& object) {
	std::list<MediaSessionImpl *>::iterator it;
	MediaSessionImpl *ms;
	bool found = FALSE;

	mutex.lock();
	for (it=sessions.begin(); !found && (it!=sessions.end()); it++) {
		if (object == (*it)->object) {
			found = true;
			ms = *it;
		}
	}
	mutex.unlock();

	if (!found) {
		throw MediaSessionNotFoundException();
	}

	return *ms;
}
