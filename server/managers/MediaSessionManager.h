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

		MediaSessionImpl& createMediaSession(const SessionSpec &spec);

		MediaSessionImpl& getMediaSession(const MediaSession& session);

		void deleteMediaSession(const MediaSession& session);

		static MediaSessionManager *getInstance();

		static void releaseInstance(MediaSessionManager *manager);

	private:

		Glib::Mutex mutex; // Protects the list
		std::map<ObjectId, MediaSessionImpl *> sessions;
	};


}}} // com::kurento::kms::api

#endif /* MEDIA_SESSION_MANAGER */