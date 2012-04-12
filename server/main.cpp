#include "handlers/MediaServiceHandler.h"
#include "handlers/MediaSessionServiceHandler.h"

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <server/TThreadedServer.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include <glibmm.h>

#include "log.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::com::kurento::kms::api;

using boost::shared_ptr;
using com::kurento::kms::MediaServerServiceHandler;
using com::kurento::kms::MediaSessionServiceHandler;
using com::kurento::kms::api::ServerConfig;
using ::com::kurento::log::Log;

static Log l("main");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

#define SERVER_SERVICE_PORT 9090
#define SESSION_SERVICE_PORT 9091

static ServerConfig config;

static void create_server_service() {
	int port;

	if (!config.__isset.serverServicePort) {
		e("No port set in configuration for MediaServerService");
		throw Glib::Thread::Exit();
	} else {
		port = config.serverServicePort;
	}

	shared_ptr<MediaServerServiceHandler> handler(new MediaServerServiceHandler(&config));
	shared_ptr<TProcessor> processor(new MediaServerServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting MediaServerService");
	server->serve();

	i("MediaServerService stopped finishing thread");
	throw Glib::Thread::Exit();
}

static void create_session_service() {
	int port;

	if (!config.__isset.mediaSessionServicePort) {
		w("No port set in configuration for MediaSessionService");
		throw Glib::Thread::Exit();
	} else {
		port = config.mediaSessionServicePort;
	}

	shared_ptr<MediaSessionServiceHandler> handler(new MediaSessionServiceHandler());
	shared_ptr<TProcessor> processor(new MediaSessionServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting MediaSessionService");
	server->serve();


	i("MediaSessionService stopped finishing thread");
	throw Glib::Thread::Exit();
}

int main(int argc, char **argv) {

	Glib::thread_init();

	config.__set_serverServicePort(SERVER_SERVICE_PORT);
	config.__set_mediaSessionServicePort(SESSION_SERVICE_PORT);

	sigc::slot<void> ss = sigc::ptr_fun(&create_server_service);
	Glib::Thread *serverServiceThread = Glib::Thread::create(ss, true);

	sigc::slot<void> mss = sigc::ptr_fun(&create_session_service);
	Glib::Thread::create(mss, true);

	Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create(true);
	loop->run();
	serverServiceThread->join();

	// TODO: Finish all other threads and notify error

	return 0;
}
