#ifndef MEDIA_SESSION_SERVICE_HANDLER
#define MEDIA_SESSION_SERVICE_HANDLER

#include "MediaSessionService.h"
#include "managers/MediaSessionManager.h"

using ::com::kurento::kms::api::MediaSessionServiceIf;
using ::com::kurento::kms::api::MediaObject;
using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::NetworkConnectionConfig;
using ::com::kurento::kms::api::Mixer;
using ::com::kurento::kms::api::MixerConfig;

namespace com { namespace kurento { namespace kms {

	class MediaSessionServiceHandler : virtual public MediaSessionServiceIf {
	public:
		MediaSessionServiceHandler();
		~MediaSessionServiceHandler();

		void createNetworkConnection(NetworkConnection& _return, const MediaSession& mediaSessionId, const std::vector<NetworkConnectionConfig::type>& config);
		void deleteNetworkConnection(const MediaSession& mediaSessionId, const NetworkConnection& networConnection);
		void getNetworkConnections(std::vector<NetworkConnection>& _return, const MediaSession& mediaSessionId);
		void createMixer(Mixer& _return, const MediaSession& mediaSessionId, const std::vector<MixerConfig::type>& config);
		void deleteMixer(const MediaSession& mediaSessionId, const  Mixer& mixer);
		void getMixers(std::vector<Mixer>& _return, const MediaSession& mediaSessionId);

		void ping(const MediaObject& mediaObject, const int32_t timeout);
		void release(const MediaObject& mediaObject);

	private:
		MediaSessionManager *manager;
	};

}}} // com::kurento::kms::api

#endif /* MEDIA_SERVER_SERVICE */