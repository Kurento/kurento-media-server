#ifndef MEDIA_SESSION_MANAGER
#define MEDIA_SESSION_MANAGER

#include "mediaSession_types.h"
#include "types/MediaSessionImpl.h"
#include <glibmm.h>

using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::ObjectId;
using ::boost::shared_ptr;

namespace com { namespace kurento { namespace kms {

	class MediaSessionManager {
	public:
		MediaSessionManager();
		~MediaSessionManager();

		MediaSessionImpl& createMediaSession();

		MediaSessionImpl& getMediaSession(const MediaSession& session);

		MediaSessionImpl& getMediaSession(const MediaObject& session);

		void deleteMediaSession(const MediaSession& session);

		void deleteMediaSession(const MediaObject& session);

		static MediaSessionManager *getInstance();

		static void releaseInstance(MediaSessionManager *manager);

	private:

		Glib::Mutex mutex; // Protects the list
		std::list<MediaSessionImpl *> sessions;
	};


}}} // com::kurento::kms::api

#endif /* MEDIA_SESSION_MANAGER */