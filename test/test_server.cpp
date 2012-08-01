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

#include <MediaServerService.h>
#include <MediaSessionService.h>
#include <NetworkConnectionService.h>
#include <thrift/config.h>
#include <thrift/transport/TSocket.h>
// #include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
// #include <thrift/protocol/TDebugProtocol.h>
#include <boost/concept_check.hpp>

#include <gst/gst.h>

#include <sstream>

using ::com::kurento::kms::api::MediaSessionServiceClient;
using ::com::kurento::kms::api::MediaServerServiceClient;
using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::ServerConfig;
using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::NetworkConnectionConfig;
using ::com::kurento::kms::api::NetworkConnectionServiceClient;
using ::com::kurento::kms::api::StreamType;
using ::com::kurento::mediaspec::SessionSpec;
using ::com::kurento::mediaspec::MediaSpec;
using ::com::kurento::mediaspec::Payload;
using ::com::kurento::mediaspec::Direction;
using ::com::kurento::mediaspec::MediaType;
using ::apache::thrift::protocol::TBinaryProtocol;
// using ::apache::thrift::protocol::TDebugProtocol;
using ::apache::thrift::transport::TSocket;
// using ::apache::thrift::transport::TMemoryBuffer;

#define DEFAULT_PORT 5050
#define LOCAL_ADDRESS "193.147.51.16"

static void
send_receive_media(SessionSpec &spec) {
	MediaSpec media = *(spec.medias.begin());
	Payload pay = *(media.payloads.begin());

	std::stringstream stream;

	if (media.direction == Direction::RECVONLY) {
		std::cout << "server recvonly" << std::endl;
		std::cout << "Port: " << media.transport.rtp.port << std::endl;
		std::cout << "Codec name: " << pay.rtp.codecName << std::endl;

		stream << "autovideosrc ! "
			"video/x-raw-yuv,framerate=30/1,width=352 ! "
			"xvidenc max-bquant=0 bquant-ratio=0 motion=0 ! "
			"rtpmp4vpay config-interval=2 send-config=TRUE ! "
			"application/x-rtp,media=video,clock-rate=(int)" <<
			pay.rtp.clockRate << ",encoding-name=(string)MP4V-ES,"
			"payload=(int)" << pay.rtp.id << " ! udpsink port=" <<
			media.transport.rtp.port << " host=" <<
			media.transport.rtp.address;
	} else if (media.direction == Direction::SENDONLY) {
		std::cout << "server sendonly" << std::endl;
		std::cout << "Port: " << DEFAULT_PORT << std::endl;
		std::cout << "Codec name: " << pay.rtp.codecName << std::endl;

		stream << "udpsrc port=" << DEFAULT_PORT <<
				" ! application/x-rtp,encoding-name=" <<
				pay.rtp.codecName << ",clock-rate=" <<
				pay.rtp.clockRate << ",payload=" <<
				pay.rtp.id << " ! rtph264depay ! ffdec_h264 ! "
				"autovideosink";
	}

	GstElement *pipe;
	GError *err = NULL;

	std::string pipe_str = stream.str();
	std::cout << pipe_str << std::endl;

	pipe = gst_parse_launch(pipe_str.c_str(), &err);
	if (err != NULL) {
		std::cout << "Error: " << err->message << std::endl;
		g_error_free(err);
		err = NULL;
	} else if (pipe == NULL) {
		std::cout << "Cannot create pipe" << std::endl;
	} else {
		gst_element_set_state(pipe, GST_STATE_PLAYING);
	}

	std::cout << "---" << std::endl;
}

static void
print_session_spec(SessionSpec &spec) {
// 	boost::shared_ptr<TMemoryBuffer> buffer(new TMemoryBuffer());
// 	TDebugProtocol proto(buffer);
// 	spec.write(&proto);
//
// 	std::cout << buffer->getBufferAsString() << std::endl;
}

static void
create_session_spec(SessionSpec &spec) {
	Payload pay;

	pay.__isset.rtp = true;
	pay.rtp.codecName = "H264";
	pay.rtp.id = 96;
	pay.rtp.clockRate = 90000;

	MediaSpec ms;

	ms.transport.rtp.address = LOCAL_ADDRESS;
	ms.transport.rtp.port = DEFAULT_PORT;
	ms.transport.__isset.rtp = true;
	ms.direction = Direction::type::RECVONLY;
	ms.type.insert(MediaType::VIDEO);
	ms.payloads.push_back(pay);

	spec.id = "1234";
	spec.medias.push_back(ms);
}

static void
create_second_session_spec(SessionSpec &spec) {
	Payload pay;

	pay.__isset.rtp = true;
	pay.rtp.codecName = "MP4V-ES";
	pay.rtp.id = 96;
	pay.rtp.clockRate = 90000;

	MediaSpec ms;

	ms.transport.rtp.address = LOCAL_ADDRESS;
	ms.transport.rtp.port = DEFAULT_PORT;
	ms.transport.__isset.rtp = true;
	ms.direction = Direction::type::SENDONLY;
	ms.type.insert(MediaType::VIDEO);
	ms.payloads.push_back(pay);

	spec.id = "1234";
	spec.medias.push_back(ms);
}

static void
create_newtork_connection(NetworkConnection &_nc, ServerConfig &mc, MediaSession &ms) {
	if (!mc.__isset.mediaSessionServicePort)
		return;

	boost::shared_ptr<TSocket> transport(new TSocket(mc.address,
						mc.mediaSessionServicePort));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	MediaSessionServiceClient mservice(prot);
	transport->open();

	std::vector<NetworkConnectionConfig::type> config;
	config.push_back(NetworkConnectionConfig::type::RTP);

	mservice.createNetworkConnection(_nc, ms, config);
}

static void
negotiate_connection(ServerConfig &mc, NetworkConnection& nc, SessionSpec& spec) {
	if (!mc.__isset.networkConnectionServicePort)
		return;

	boost::shared_ptr<TSocket> transport(new TSocket(mc.address,
					mc.networkConnectionServicePort));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	NetworkConnectionServiceClient service(prot);
	transport->open();

	SessionSpec reply;
	service.processOffer(reply, nc, spec);

	print_session_spec(reply);
	send_receive_media(reply);
}

static void
join_network_connections(ServerConfig &mc, NetworkConnection& nc,
						NetworkConnection& nc2) {
	if (!mc.__isset.networkConnectionServicePort)
		return;

	boost::shared_ptr<TSocket> transport(new TSocket(mc.address,
							 mc.networkConnectionServicePort));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	NetworkConnectionServiceClient service(prot);
	transport->open();

	service.joinStream(nc.joinable, nc2.joinable, StreamType::type::VIDEO,
							Direction::RECVONLY);
}

static void
ping_media_session(ServerConfig &mc, MediaSession& ms, int iter, int interval) {
	if (!mc.__isset.mediaSessionServicePort)
		return;

	boost::shared_ptr<TSocket> transport(new TSocket(mc.address,
							 mc.mediaSessionServicePort));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	MediaSessionServiceClient service(prot);
	transport->open();

	for (; iter > 0; iter--) {
		service.ping(ms, interval + 1);
		sleep(interval);
	}
}

static void
start_client() {
	boost::shared_ptr<TSocket> transport(new TSocket("localhost", 9090));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	MediaServerServiceClient mservice(prot);

	transport->open();

	ServerConfig mc;
	MediaSession ms;
	NetworkConnection nc, nc2;
	SessionSpec spec;
	SessionSpec spec2;

	mservice.getServerconfig(mc);
	mservice.createMediaSession(ms);
	std::cout << "Session id: " << ms.object.id << std::endl;

 	create_newtork_connection(nc, mc, ms);
	std::cout << "NetworkConnection: " << nc.joinable.object.id << std::endl;
	create_session_spec(spec);
	negotiate_connection(mc, nc, spec);

	create_newtork_connection(nc2, mc, ms);
	std::cout << "NetworkConnection: " << nc2.joinable.object.id << std::endl;
	create_second_session_spec(spec2);
	negotiate_connection(mc, nc2, spec2);

	join_network_connections(mc, nc, nc2);

	ping_media_session(mc, ms, 4, 30);

	mservice.deleteMediaSession(ms);
}

int
main(int argc, char **argv) {
	gst_init(&argc, &argv);
	// TODO: Evaluate if server should be launched here

	start_client();
}