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
