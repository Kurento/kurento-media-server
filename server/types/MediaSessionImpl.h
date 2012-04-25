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
	MediaSessionImpl(const SessionSpec &spec);
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

private:

	NetworkConnectionManager ncManager;
	MixerManager mixerManager;
	sigc::connection conn;

	bool pingTimeout();
};

}}} // com::kurento::kms

#endif /* MEDIA_SESSION_IMPL */
