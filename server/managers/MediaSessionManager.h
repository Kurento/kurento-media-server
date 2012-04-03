#ifndef MEDIA_SESSION_MANAGER
#define MEDIA_SESSION_MANAGER

#include "mediaSession_types.h"

using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::ObjectId;
using ::boost::shared_ptr;

namespace com { namespace kurento { namespace kms {

	class MediaSessionManager {
	public:
		MediaSessionManager();
		~MediaSessionManager();

		MediaSession& createMediaSession();

		MediaSession& getMediaSession(ObjectId id);

		void deleteMediaSession(const MediaSession& session);

		static MediaSessionManager *getInstance();

		static void releaseInstance(MediaSessionManager *manager);

	private:


	};


}}} // com::kurento::kms::api

#endif /* MEDIA_SESSION_MANAGER */