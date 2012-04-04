#ifndef MEDIA_SESSION_IMPL
#define MEDIA_SESSION_IMPL

#include "mediaSession_types.h"
#include "MediaSessionService.h"
#include "types/MediaObjectImpl.h"

using namespace ::com::kurento::kms::api;

namespace com { namespace kurento { namespace kms {

class MediaSessionImpl : public MediaSession,
			public virtual MediaObjectImpl,
			public virtual MediaSessionServiceIf {
public:
	MediaSessionImpl();
	~MediaSessionImpl() throw() {};

	void createNetworkConnection(NetworkConnection& _return, const MediaSession& mediaSession, const std::vector<NetworkConnectionConfig::type> & config);
	void deleteNetworkConnection(const MediaSession& mediaSession, const NetworkConnection& networConnection);
	void getNetworkConnections(std::vector<NetworkConnection> & _return, const MediaSession& mediaSession);
	void createMixer(Mixer& _return, const MediaSession& mediaSession, const std::vector<MixerConfig::type> & config);
	void deleteMixer(const MediaSession& mediaSession, const Mixer& mixer);
	void getMixers(std::vector<Mixer> & _return, const MediaSession& mediaSession);

};

}}} // com::kurento::kms

#endif /* MEDIA_SESSION_IMPL */