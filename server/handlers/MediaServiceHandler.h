#include "MediaServerService.h"
#include "managers/MediaSessionManager.h"

#ifndef MEDIA_SERVER_SERVICE
#define MEDIA_SERVER_SERVICE

using ::com::kurento::kms::api::MediaServerServiceIf;

namespace com { namespace kurento { namespace kms {

class MediaServerServiceHandler : virtual public MediaServerServiceIf {
public:
	MediaServerServiceHandler();
	~MediaServerServiceHandler();

	void createMediaSession(api::MediaSession& _return);

	void deleteMediaSession(const api::MediaSession& session);

private:
	MediaSessionManager *manager;
};

}}} // com::kurento::kms::api

#endif /* MEDIA_SERVER_SERVICE */
