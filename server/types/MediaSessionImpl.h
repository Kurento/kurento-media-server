#ifndef MEDIA_SESSION_IMPL
#define MEDIA_SESSION_IMPL

#include "mediaSession_types.h"
#include "types/MediaObjectImpl.h"
#include "managers/NetworkConnectionManager.h"

using namespace ::com::kurento::kms::api;
using ::com::kurento::kms::NetworkConnectionManager;

namespace com { namespace kurento { namespace kms {

class MediaSessionImpl : public MediaSession,
			public virtual MediaObjectImpl {
public:
	MediaSessionImpl();
	~MediaSessionImpl() throw();

	NetworkConnection &createNetworkConnection(const std::vector<NetworkConnectionConfig::type> & config);
	void deleteNetworkConnection(const NetworkConnection& networConnection);
	std::vector<NetworkConnection> &getNetworkConnections();
	Mixer &createMixer(const std::vector<MixerConfig::type> & config);
	void deleteMixer(const Mixer& mixer);
	std::vector<Mixer> &getMixers();

private:

	NetworkConnectionManager *ncManager;
};

}}} // com::kurento::kms

#endif /* MEDIA_SESSION_IMPL */
