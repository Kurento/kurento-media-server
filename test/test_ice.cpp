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

#include <nice.h>
#include <glib.h>
#include <gst/gst.h>

#include <MediaServerService.h>
#include <MediaSessionService.h>
#include <NetworkConnectionService.h>

#include <boost/shared_ptr.hpp>
#include <boost/concept_check.hpp>
#include <Thrift.h>
#include <transport/TSocket.h>
#include <protocol/TBinaryProtocol.h>
#include <iostream>

// #define KMS_SERVER "127.0.0.1"
#define KMS_SERVER "193.147.51.22"

using ::com::kurento::kms::api::ServerConfig;
using ::com::kurento::kms::api::MediaServerServiceClient;
using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::NetworkConnectionServiceClient;
using ::com::kurento::kms::api::NetworkConnectionConfig;
using ::com::kurento::kms::api::MediaSessionServiceClient;

using ::apache::thrift::transport::TSocket;
using ::apache::thrift::protocol::TBinaryProtocol;

GMainContext *ctx;

static void
create_remote_endpoint(NetworkConnection &_nc, ServerConfig &sc, MediaSession &ms) {

	if (!sc.__isset.mediaSessionServicePort)
		return;

	boost::shared_ptr<TSocket> transport(new TSocket(sc.address,
						sc.mediaSessionServicePort));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	MediaSessionServiceClient mservice(prot);
	transport->open();

	std::vector<NetworkConnectionConfig::type> config;
	config.push_back(NetworkConnectionConfig::type::RTP);

	mservice.createNetworkConnection(_nc, ms, config);
}

static void
create_media_session(MediaSession &_ms, ServerConfig &_sc, const char *server) {
	boost::shared_ptr<TSocket> transport(new TSocket(server, 9090));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	MediaServerServiceClient mservice(prot);

	transport->open();

	mservice.getServerconfig(_sc);

	mservice.createMediaSession(_ms);

	std::cout << "MediaSession: " << _ms.object.id << std::endl;

	transport->close();
}

static void
delete_media_session(const char *server, MediaSession &ms) {
	boost::shared_ptr<TSocket> transport(new TSocket(server, 9090));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	MediaServerServiceClient mservice(prot);

	transport->open();

	mservice.deleteMediaSession(ms);

	transport->close();
}

int
main(int argc, char **argv) {
	NiceAgent *agent;
	std::string server;
	MediaSession ms;
	ServerConfig sc;
	NetworkConnection nc;

	gst_init(&argc, &argv);

	ctx = g_main_context_default();

	std::cout << "Creating agent" << std::endl;

	agent = nice_agent_new(ctx, NICE_COMPATIBILITY_RFC5245);
	server = KMS_SERVER;

	create_media_session(ms, sc, server.c_str());
	create_remote_endpoint(nc, sc, ms);


	delete_media_session(server.c_str(), ms);
	std::cout << "Destroying agent" << std::endl;
	g_object_unref(agent);
}
