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

		void createNetworkConnection(NetworkConnection& _return, const MediaSession& mediaSession, const std::vector<NetworkConnectionConfig::type>& config);
		void deleteNetworkConnection(const MediaSession& mediaSession, const NetworkConnection& networkConnection);
		void getNetworkConnections(std::vector<NetworkConnection>& _return, const MediaSession& mediaSession);
		void createMixer(Mixer& _return, const MediaSession& mediaSession, const std::vector<MixerConfig::type>& config);
		void deleteMixer(const MediaSession& mediaSession, const  Mixer& mixer);
		void getMixers(std::vector<Mixer>& _return, const MediaSession& mediaSession);

		void ping(const MediaObject& mediaObject, const int32_t timeout);
		void release(const MediaObject& mediaObject);

	private:
		MediaSessionManager *manager;
	};

}}} // com::kurento::kms::api

#endif /* MEDIA_SERVER_SERVICE */