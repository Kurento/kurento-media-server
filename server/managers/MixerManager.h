#ifndef MIXER_MANAGER
#define MIXER_MANAGER

#include "types/MixerImpl.h"
#include <glibmm.h>

using ::com::kurento::kms::api::Mixer;

namespace com { namespace kurento { namespace kms {

	class MixerManager {
	public:
		MixerManager();
		~MixerManager();

		MixerImpl& createMixer(MediaSession &session, const std::vector<MixerConfig::type> &config);

		void getMixers(std::vector<Mixer> &_return);
		MixerImpl& getMixer(const Mixer& mixer);
		void deleteMixer(const Mixer& mixer);

	private:

		Glib::Mutex mutex; // Protects the list
		std::map<ObjectId, MixerImpl *> mixers;
	};


}}} // com::kurento::kms::api

#endif /* MIXER_MANAGER */
