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
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TDebugProtocol.h>
#include <iostream>

// #define KMS_SERVER "127.0.0.1"
#define KMS_SERVER "193.147.51.25"

using ::com::kurento::kms::api::ServerConfig;
using ::com::kurento::kms::api::MediaServerServiceClient;
using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::NetworkConnectionServiceClient;
using ::com::kurento::kms::api::NetworkConnectionConfig;
using ::com::kurento::kms::api::MediaSessionServiceClient;
using ::com::kurento::kms::api::NetworkConnectionServiceClient;

using ::com::kurento::mediaspec::SessionSpec;
using ::com::kurento::mediaspec::MediaSpec;
using ::com::kurento::mediaspec::Payload;
using ::com::kurento::mediaspec::Direction;
using ::com::kurento::mediaspec::MediaType;
using ::com::kurento::mediaspec::TransportIceCandidate;
using ::com::kurento::mediaspec::TransportIceCandidateTransport;
using ::com::kurento::mediaspec::TransportIceCandidateType;

using ::apache::thrift::transport::TSocket;
using ::apache::thrift::transport::TMemoryBuffer;
using ::apache::thrift::protocol::TBinaryProtocol;
using ::apache::thrift::protocol::TDebugProtocol;

GMainContext *ctx;
GMainLoop *loop;
GMutex mutex;
GCond cond;
gboolean gathered = false;

static void
print_spec(SessionSpec &spec) {
	boost::shared_ptr<TMemoryBuffer> buffer(new TMemoryBuffer());
	TDebugProtocol proto(buffer);
	spec.write(&proto);
	std::cout << "Spec:\n" << buffer->getBufferAsString() << std::endl;
}

static void
cb_candidate_gathering_done(NiceAgent *agent, guint stream_id, gpointer data) {
	g_mutex_lock(&mutex);
	gathered = true;
	g_cond_signal(&cond);
	g_mutex_unlock(&mutex);
}

static void
create_session_spec(SessionSpec &spec, NiceAgent *agent) {
	Payload pay;

	pay.__isset.rtp = true;
	pay.rtp.codecName = "MP4V-ES";
	pay.rtp.id = 97;
	pay.rtp.clockRate = 90000;
	pay.rtp.__set_width(320);
	pay.rtp.__set_height(240);
	pay.rtp.__set_bitrate(200);

	MediaSpec ms;

	guint stream_id = nice_agent_add_stream(agent, 1);
	g_object_set_data(G_OBJECT(agent), "stream", GUINT_TO_POINTER(stream_id));

	g_object_set(G_OBJECT(agent), "stun-server", "193.147.51.24", NULL);
	g_object_set(G_OBJECT(agent), "stun-server-port", (guint) 3478 , NULL);

	g_signal_connect(G_OBJECT(agent), "candidate-gathering-done",
				G_CALLBACK(cb_candidate_gathering_done), NULL);

	if (!nice_agent_gather_candidates(agent, stream_id)) {
		std::cout << "Error getting candidates" << std::endl;
		throw "Error getting candidates";
	}

	g_mutex_lock(&mutex);
	if(!gathered) {
		gint64 end_time = g_get_monotonic_time () +
		10 * G_TIME_SPAN_SECOND;
		g_cond_wait_until(&cond, &mutex, end_time);
		if (!gathered) {
			std::cout << "Timeout waiting for candidates" << std::endl;
			throw "Timeout waiting for candidates";
		}
	}
	g_mutex_unlock(&mutex);

	GSList *cands, *l;
	gchar aux[NICE_ADDRESS_STRING_LEN];
	guint32 priority = 0;
	gchar public_addr[NICE_ADDRESS_STRING_LEN];
	guint public_port;

	cands = nice_agent_get_local_candidates(agent, stream_id, 1);

	for (l = cands; l != NULL; l = l->next) {
		NiceCandidate *cand = (NiceCandidate *) l->data;

		nice_address_to_string(&cand->addr, aux);

		if (g_strrstr(aux, ":") != NULL)
			continue;

		TransportIceCandidate ic;

		ic.address = aux;
		ic.port = nice_address_get_port(&cand->addr);
		nice_address_to_string(&cand->base_addr, aux);
		ic.baseAddress = aux;
		ic.__isset.baseAddress = true;
		ic.basePort = nice_address_get_port(&cand->base_addr);
		ic.__isset.basePort = true;
		ic.componentId = 1;
		ic.foundation.assign(cand->foundation);
		ic.priority = cand->priority;
		ic.streamId = cand->stream_id;
		ic.__isset.streamId = stream_id;

		switch (cand->transport) {
		case NICE_CANDIDATE_TRANSPORT_UDP:
			ic.transport = TransportIceCandidateTransport::UDP;
			break;
		}

		switch (cand->type) {
		case NICE_CANDIDATE_TYPE_HOST:
			ic.type = TransportIceCandidateType::HOST;
			break;
		case NICE_CANDIDATE_TYPE_SERVER_REFLEXIVE:
			ic.type = TransportIceCandidateType::SERVER_REFLEXIVE;
			break;
		case NICE_CANDIDATE_TYPE_PEER_REFLEXIVE:
			ic.type = TransportIceCandidateType::PEER_REFLEXIVE;
			break;
		case NICE_CANDIDATE_TYPE_RELAYED:
			ic.type = TransportIceCandidateType::RELAYED;
			break;
		}

		gchar *uname, *pass;

		nice_agent_get_local_credentials(agent, stream_id, &uname,
									&pass);

		ic.username.assign(uname);
		ic.password.assign(pass);

		g_free(pass);
		g_free(uname);

		ms.transport.ice.candidates.push_back(ic);
		ms.transport.__isset.ice = true;

		if (priority == 0  &&
			!nice_address_equal(&cand->addr, &cand->base_addr)) {
			std::cout << "1" << std::endl;
			priority = cand->priority;
			nice_address_to_string(&cand->addr, public_addr);
			public_port = nice_address_get_port(&cand->addr);
		}
	}

	g_slist_free_full(cands, (GDestroyNotify) nice_candidate_free);

	if (priority != 0) {
		ms.transport.rtp.address.assign(public_addr);
		ms.transport.rtp.port = public_port;
		ms.transport.__isset.rtp = true;
	}

	ms.direction = Direction::type::RECVONLY;
	ms.type.insert(MediaType::VIDEO);
	ms.payloads.push_back(pay);

	spec.id = "1234";
	spec.medias.push_back(ms);
}

static void
send_offer(SessionSpec &_answer, NetworkConnection &nc, SessionSpec &offer,
							ServerConfig &sc) {
	if (!sc.__isset.networkConnectionServicePort)
		return;

	boost::shared_ptr<TSocket> transport(new TSocket(sc.address,
					sc.networkConnectionServicePort));
	boost::shared_ptr<TBinaryProtocol> prot(new TBinaryProtocol(transport));
	NetworkConnectionServiceClient ncservice(prot);
	transport->open();

	ncservice.processOffer(_answer, nc, offer);

	transport->close();
}

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

static gpointer
main_loop_thread(gpointer data) {
	g_main_loop_run(loop);

	return NULL;
}

int
main(int argc, char **argv) {
	NiceAgent *agent;
	std::string server;
	MediaSession ms;
	ServerConfig sc;
	NetworkConnection nc;
	GThread *thread;
	SessionSpec spec, answer;

	gst_init(&argc, &argv);

	ctx = g_main_context_default();
	loop = g_main_loop_new(ctx, TRUE);

	thread = g_thread_new("MainLoop", main_loop_thread, NULL);

	std::cout << "Creating agent" << std::endl;

	agent = nice_agent_new(ctx, NICE_COMPATIBILITY_RFC5245);
	server = KMS_SERVER;

	create_media_session(ms, sc, server.c_str());
	create_remote_endpoint(nc, sc, ms);
	create_session_spec(spec, agent);
	send_offer(answer, nc, spec, sc);
	print_spec(spec);
	print_spec(answer);

// 	g_thread_join(thread);
	g_thread_unref(thread);

	delete_media_session(server.c_str(), ms);
	std::cout << "Destroying agent" << std::endl;
	g_object_unref(agent);
	g_main_loop_unref(loop);
}
