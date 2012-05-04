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

#ifndef NETWORK_CONNECTION_SERVICE_HANDLER
#define NETWORK_CONNECTION_SERVICE_HANDLER

#include "NetworkConnectionService.h"
#include "JoinableServiceHandler.h"

using ::com::kurento::kms::api::NetworkConnectionServiceIf;
using ::com::kurento::kms::api::MediaObject;
using ::com::kurento::commons::mediaspec::SessionSpec;
using ::com::kurento::kms::api::NetworkConnection;

namespace com { namespace kurento { namespace kms {

class NetworkConnectionServiceHandler :
				virtual public JoinableServiceHandler,
				virtual public NetworkConnectionServiceIf {
public:
	NetworkConnectionServiceHandler();
	~NetworkConnectionServiceHandler() {};

	void generateOffer(SessionSpec &_return, const NetworkConnection &nc);
	void processAnswer(SessionSpec &_return, const NetworkConnection &nc, const SessionSpec &anwser);
	void processOffer(SessionSpec &_return, const NetworkConnection &nc, const SessionSpec &offer);
	void getLocalDescriptor(SessionSpec &_return, const NetworkConnection &nc);
	void getRemoteDescriptor(SessionSpec &_return, const NetworkConnection &nc);
};

}}} // com::kurento::kms::api

#endif /* NETWORK_CONNECTION_SERVICE_HANDLER */
