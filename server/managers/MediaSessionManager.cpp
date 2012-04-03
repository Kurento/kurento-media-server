#include "MediaSessionManager.h"

using namespace ::com::kurento::kms;
using namespace ::com::kurento::kms::api;

void task() {
}

MediaSessionManager::MediaSessionManager() {
}

MediaSessionManager::~MediaSessionManager() {
}

MediaSessionManager *MediaSessionManager::getInstance() {
	static shared_ptr<MediaSessionManager> instance(new MediaSessionManager());
	return instance.get();
}

void MediaSessionManager::releaseInstance(MediaSessionManager* manager) {
	// As instance is a singleton no action is needed
}

MediaSession& MediaSessionManager::createMediaSession() {
	MediaServerException exception;
	exception.__set_description("Not implemented");
	exception.__set_code(ErrorCode::UNEXPECTED);
	throw exception;
}

void MediaSessionManager::deleteMediaSession(const MediaSession& session) {
	MediaServerException exception;
	exception.__set_description("Not implemented");
	exception.__set_code(ErrorCode::UNEXPECTED);
	throw exception;
}
