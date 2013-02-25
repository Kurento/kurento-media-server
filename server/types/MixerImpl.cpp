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

#include "types/MixerImpl.h"

#include <log.h>

using ::com::kurento::kms::MixerImpl;
using ::com::kurento::kms::api::MixerConfig;

using ::com::kurento::log::Log;

static Log l("MixerImpl");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);


MixerImpl::MixerImpl(MediaSession &session,
			const std::vector<MixerConfig::type> & config) :
					JoinableImpl(session), Mixer() {
	__set_joinable(*this);

	__set_config(config);
}

MixerImpl::~MixerImpl() throw () {
}
