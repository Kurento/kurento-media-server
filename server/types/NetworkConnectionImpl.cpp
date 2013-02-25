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

#include "types/NetworkConnectionImpl.h"

#include <log.h>

using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
using ::com::kurento::kms::api::NetworkConnection;

using ::com::kurento::log::Log;

static Log l("NetworkConnectionImpl");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

NetworkConnectionImpl::NetworkConnectionImpl(MediaSession &session,
				const std::vector<NetworkConnectionConfig::type> & config,
				const std::string &spec) :
				JoinableImpl(session), NetworkConnection() {
	
}

NetworkConnectionImpl::~NetworkConnectionImpl() throw() {
	
}

void
NetworkConnectionImpl::generateOffer(std::string& _return) {
	
}

void
NetworkConnectionImpl::processAnswer(std::string &_return, const std::string &answer) {
	
}

void
NetworkConnectionImpl::processOffer(std::string& _return, const std::string& offer) {
	
}

void
NetworkConnectionImpl::getLocalDescriptor(std::string& _return) {
	
}

void
NetworkConnectionImpl::getRemoteDescriptor(std::string& _return) {
	
}
