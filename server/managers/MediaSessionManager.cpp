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
	sessions[session->object.id] = session;
	mutex.unlock();

	return *session;
}

void MediaSessionManager::deleteMediaSession(const MediaSession &session) {
	deleteMediaSession(session.object);
}

void MediaSessionManager::deleteMediaSession(const MediaObject &object) {
	std::map<ObjectId, MediaSessionImpl *>::iterator it;
	bool found = FALSE;

	mutex.lock();
	it = sessions.find(object.id);
	if (it != sessions.end() && object == it->second->object) {
		found = true;
		delete it->second;
		sessions.erase(it);
	} else {
		found = false;
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
	std::map<ObjectId, MediaSessionImpl *>::iterator it;
	MediaSessionImpl *ms;
	bool found = FALSE;

	mutex.lock();
	it = sessions.find(object.id);
	if (it != sessions.end() && object == it->second->object) {
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
