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

#include "types/JoinableImpl.h"

#include <glibmm.h>

#include <log.h>

using ::com::kurento::kms::MediaObjectImpl;
using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::Joinable;
using ::com::kurento::kms::api::MediaServerException;
using ::com::kurento::kms::api::ErrorCode;
using ::com::kurento::kms::api::JoinException;
using ::com::kurento::kms::api::StreamNotFoundException;

using ::com::kurento::log::Log;

static Log l("JoinableImpl");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

JoinableImpl::JoinableImpl(MediaSession &session) :
					MediaObjectImpl(session.object.token),
					Joinable() {
	__set_object(*this);
	__set_session(session);
}

JoinableImpl::~JoinableImpl() throw () {
	
}
void
JoinableImpl::getStreams(std::vector<StreamType::type> &_return) {
	
}

void
JoinableImpl::join(const JoinableImpl& to, const Direction::type direction) {
	
}

void
JoinableImpl::unjoin(JoinableImpl& to) {
	
}

void
JoinableImpl::join(JoinableImpl &to, const StreamType::type stream, const Direction::type direction) {
	
}

void
JoinableImpl::unjoin(JoinableImpl &to, const StreamType::type stream) {
	
}

void
JoinableImpl::getJoinees(std::vector<Joinable> &_return) {
	
}

void
JoinableImpl::getJoinees(std::vector<Joinable> &_return, const Direction::type direction) {
	
}

void
JoinableImpl::getJoinees(std::vector<Joinable> &_return, const StreamType::type stream) {
	
}

void
JoinableImpl::getJoinees(std::vector<Joinable> &_return, const StreamType::type stream, const Direction::type direction) {
	
}
