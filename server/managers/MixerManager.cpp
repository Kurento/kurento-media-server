#include "MixerManager.h"

using ::com::kurento::kms::MixerManager;
using ::com::kurento::kms::MixerImpl;

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

MixerImpl& MixerManager::createMixer(
	MediaSession &session,
	const std::vector<MixerConfig::type> & config) {
	MixerImpl *mixer = new MixerImpl(session, config);

	mutex.lock();
	mixers.push_back(mixer);
	mutex.unlock();

	return *mixer;
}

void
MixerManager::deleteMixer(const Mixer& mixer) {
	std::vector<MixerImpl *>::iterator it;
	bool found;

	mutex.lock();
	for (it = mixers.begin(); it != mixers.end(); it++) {
		if (mixer == *(*it)) {
			found = true;
			delete *it;
			mixers.erase(it);
			break;
		}
	}
	mutex.unlock();

	if (!found) {
		MixerNotFoundException exception;
		throw exception;
	}
}
