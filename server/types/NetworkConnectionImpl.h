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

#ifndef NETWORK_CONNECTION_IMPL
#define NETWORK_CONNECTION_IMPL

#include "networkConnection_types.h"
#include "types/JoinableImpl.h"

using namespace ::com::kurento::kms::api;
using ::com::kurento::commons::mediaspec::SessionSpec;

namespace com { namespace kurento { namespace kms {

	class NetworkConnectionImpl : public NetworkConnection,
					public virtual JoinableImpl {
	public:
		NetworkConnectionImpl(MediaSession &session,
				const std::vector<NetworkConnectionConfig::type> & config,
				const SessionSpec &spec);
		~NetworkConnectionImpl() throw();

		void generateOffer(SessionSpec& _return);
		void processAnswer(SessionSpec &_return, const SessionSpec &answer);
		void processOffer(SessionSpec& _return, const SessionSpec& offer);
		void getLocalDescriptor(SessionSpec& _return);
		void getRemoteDescriptor(SessionSpec& _return);

	private:
		SessionSpec spec;
		KmsConnection *rtp_connection = NULL;
	};

}}} // com::kurento::kms

#endif /* NETWORK_CONNECTION_IMPL */
