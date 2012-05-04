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

#ifndef JOINABLE_SERVICE_HANDLER
#define JOINABLE_SERVICE_HANDLER

#include "JoinableService.h"
#include "managers/MediaSessionManager.h"

using ::com::kurento::kms::api::JoinableServiceIf;
using ::com::kurento::kms::api::Joinable;
using ::com::kurento::kms::api::StreamType;
using ::com::kurento::kms::api::MediaObject;

using ::com::kurento::kms::MediaSessionManager;

namespace com { namespace kurento { namespace kms {

class JoinableServiceHandler : virtual public JoinableServiceIf {
public:
	JoinableServiceHandler();
	virtual ~JoinableServiceHandler();

	void getStreams(std::vector<StreamType::type> & _return, const Joinable& joinable);
	void join(const Joinable& from, const Joinable& to, const Direction direction);
	void unjoin(const Joinable& from, const Joinable& to);
	void joinStream(const Joinable& from, const Joinable& to, const StreamType::type stream, const Direction direction);
	void unjoinStream(const Joinable& from, const Joinable& to, const StreamType::type stream);
	void getJoinees(std::vector<Joinable> & _return, const Joinable& from);
	void getDirectionJoinees(std::vector<Joinable> & _return, const Joinable& from, const Direction direction);
	void getStreamJoinees(std::vector<Joinable> & _return, const Joinable& from, const StreamType::type stream);
	void getStreamDirectionJoinees(std::vector<Joinable> & _return, const Joinable& from, const StreamType::type stream, const Direction direction);

	void release(const Joinable& joinable);

protected:

	MediaSessionManager *manager;
};

}}} // com::kurento::kms::api

#endif /* JOINABLE_SERVICE_HANDLER */
