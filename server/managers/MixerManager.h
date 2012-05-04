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
		JoinableImpl& getJoinable(const Joinable &joinable);
		void deleteMixer(const Mixer& mixer);
		void deleteJoinable(const Joinable &joinable);

	private:

		Glib::Mutex mutex; // Protects the list
		std::map<ObjectId, MixerImpl *> mixers;
	};


}}} // com::kurento::kms::api

#endif /* MIXER_MANAGER */
