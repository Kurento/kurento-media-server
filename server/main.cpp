#include "handlers/MediaServiceHandler.h"

#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <server/TThreadedServer.h>
#include <server/TNonblockingServer.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include <boost/thread.hpp>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::com::kurento::kms::api;

using boost::shared_ptr;
using com::kurento::kms::MediaServerServiceHandler;

static void create_server_service() {
	int port = 9090;

	shared_ptr<MediaServerServiceHandler> handler(new MediaServerServiceHandler());
	shared_ptr<TProcessor> processor(new MediaServerServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager>  threadManager=ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor, serverTransport, transportFactory, protocolFactory, threadFactory));
	server->serve();
}

int main(int argc, char **argv) {

	boost::thread serverServiceThread(create_server_service);

	// Waiting for server thread to finish

	serverServiceThread.join();

	return 0;
}
