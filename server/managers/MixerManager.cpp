#include "MixerManager.h"

using ::com::kurento::kms::MixerManager;
using ::com::kurento::kms::MixerImpl;
using ::com::kurento::kms::JoinableImpl;

MixerManager::MixerManager(){
}

MixerManager::~MixerManager() {
	std::map<ObjectId, MixerImpl *>::iterator it;

	mutex.lock();
	for (it = mixers.begin(); it != mixers.end(); it++) {
		delete it->second;
	}
	mixers.clear();
	mutex.unlock();
}

MixerImpl& MixerManager::createMixer(
	MediaSession &session,
	const std::vector<MixerConfig::type> & config) {
	MixerImpl *mixer = new MixerImpl(session, config);

	mutex.lock();
	mixers[mixer->joinable.object.id] = mixer;
	mutex.unlock();

	return *mixer;
}

void
MixerManager::deleteMixer(const Mixer& mixer) {
	std::map<ObjectId, MixerImpl *>::iterator it;
	bool found;

	mutex.lock();
	it = mixers.find(mixer.joinable.object.id);
	if (it != mixers.end() && mixer == *(it->second)) {
		found = true;
		delete it->second;
		mixers.erase(it);
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		MixerNotFoundException exception;
		throw exception;
	}
}

void
MixerManager::deleteJoinable(const Joinable& joinable) {
	std::map<ObjectId, MixerImpl *>::iterator it;
	bool found;

	mutex.lock();
	it = mixers.find(joinable.object.id);
	if (it != mixers.end() && joinable == it->second->joinable) {
		found = true;
		delete it->second;
		mixers.erase(it);
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		MixerNotFoundException exception;
		throw exception;
	}
}

void
MixerManager::getMixers(std::vector<Mixer> &_return) {
	std::map<ObjectId, MixerImpl *>::iterator it;

	mutex.lock();
	for (it = mixers.begin(); it != mixers.end(); it++) {
		_return.push_back(*(it->second));
	}
	mutex.unlock();
}

MixerImpl&
MixerManager::getMixer(const Mixer &m) {
	std::map<ObjectId, MixerImpl *>::iterator it;
	MixerImpl *mixer;
	bool found;

	mutex.lock();
	it = mixers.find(m.joinable.object.id);
	if (it != mixers.end() && m == *(it->second)) {
		found = true;
		mixer = it->second;
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		MixerNotFoundException ex;
		throw ex;
	}

	return *mixer;
}

JoinableImpl&
MixerManager::getJoinable(const Joinable &j) {
	std::map<ObjectId, MixerImpl *>::iterator it;
	MixerImpl *mixer;
	bool found;

	mutex.lock();
	it = mixers.find(j.object.id);
	if (it != mixers.end() && j == it->second->joinable) {
		found = true;
		mixer = it->second;
	} else {
		found = false;
	}
	mutex.unlock();

	if (!found) {
		MixerNotFoundException ex;
		throw ex;
	}

	return *mixer;
}
