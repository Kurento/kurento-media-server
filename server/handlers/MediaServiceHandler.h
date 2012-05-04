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

#ifndef MEDIA_SERVER_SERVICE
#define MEDIA_SERVER_SERVICE

#include "MediaServerService.h"
#include "managers/MediaSessionManager.h"

using ::com::kurento::kms::api::MediaServerServiceIf;
using ::com::kurento::kms::api::ServerConfig;

namespace com { namespace kurento { namespace kms {

class MediaServerServiceHandler : virtual public MediaServerServiceIf {
public:
	MediaServerServiceHandler(const ServerConfig &config,
						const SessionSpec &spec);
	~MediaServerServiceHandler();

	void getServerconfig(ServerConfig& _return);
	void createMediaSession(api::MediaSession& _return);
	void deleteMediaSession(const api::MediaSession& session);

private:
	MediaSessionManager *manager;
	ServerConfig config;
	SessionSpec spec;
};

}}} // com::kurento::kms::api

#endif /* MEDIA_SERVER_SERVICE */
