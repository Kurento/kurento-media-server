#include "MixerManager.h"

using ::com::kurento::kms::MixerManager;

MixerManager::MixerManager(){
}

MixerManager::~MixerManager() {
	std::vector<MixerImpl *>::iterator it;

	mutex.lock();
	for (it = mixers.begin(); it != mixers.end(); it++) {
		if (it != mixers.end()) {
			delete *it;
		}
	}
	mixers.clear();
	mutex.unlock();
}