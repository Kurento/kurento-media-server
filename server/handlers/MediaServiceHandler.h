#ifndef MEDIA_SERVER_SERVICE
#define MEDIA_SERVER_SERVICE

#include "MediaServerService.h"
#include "managers/MediaSessionManager.h"

using ::com::kurento::kms::api::MediaServerServiceIf;
using ::com::kurento::kms::api::ServerConfig;

namespace com { namespace kurento { namespace kms {

class MediaServerServiceHandler : virtual public MediaServerServiceIf {
public:
	MediaServerServiceHandler(ServerConfig *config);
	~MediaServerServiceHandler();

	void getServerconfig(ServerConfig& _return);
	void createMediaSession(api::MediaSession& _return);
	void deleteMediaSession(const api::MediaSession& session);

private:
	MediaSessionManager *manager;
	ServerConfig *config;
};

}}} // com::kurento::kms::api

#endif /* MEDIA_SERVER_SERVICE */
