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

#ifndef MEDIA_SESSION_IMPL
#define MEDIA_SESSION_IMPL

#include "mediaSession_types.h"
#include "types/MediaObjectImpl.h"
#include "managers/NetworkConnectionManager.h"
#include "managers/MixerManager.h"
#include <glibmm.h>

using namespace ::com::kurento::kms::api;
using ::com::kurento::kms::NetworkConnectionManager;

namespace com { namespace kurento { namespace kms {

class MediaSessionImpl : public MediaSession,
			public virtual MediaObjectImpl {
public:
	MediaSessionImpl(const std::string &spec);
	~MediaSessionImpl() throw();

	void ping(const int32_t timeout);

	NetworkConnection &createNetworkConnection(const std::vector<NetworkConnectionConfig::type> & config);
	void deleteNetworkConnection(const NetworkConnection& networkConnection);
	void getNetworkConnections(std::vector<NetworkConnection> &_return);
	Mixer &createMixer(const std::vector<MixerConfig::type> & config);
	void deleteMixer(const Mixer& mixer);
	void getMixers(std::vector<Mixer> &_return);

	JoinableImpl &getJoinable(const Joinable &joinable);
	NetworkConnectionImpl &getNetworkConnection(const NetworkConnection &nc);

	void deleteJoinable(const Joinable &joinable);

private:

	NetworkConnectionManager ncManager;
	MixerManager mixerManager;
	sigc::connection conn;

	bool pingTimeout();
};

}}} // com::kurento::kms

#endif /* MEDIA_SESSION_IMPL */
