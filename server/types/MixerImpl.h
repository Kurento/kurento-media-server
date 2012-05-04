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

#ifndef MIXER_IMPL
#define MIXER_IMPL

#include "mixer_types.h"
#include "types/JoinableImpl.h"

using namespace ::com::kurento::kms::api;

namespace com { namespace kurento { namespace kms {

	class MixerImpl : public Mixer, public virtual JoinableImpl {
	public:
		MixerImpl(MediaSession &session, const std::vector<MixerConfig::type> &config);
		~MixerImpl() throw();

	};

}}} // com::kurento::kms

#endif /* MIXER_IMPL */