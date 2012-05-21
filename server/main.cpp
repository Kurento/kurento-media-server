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

#include <signal.h>
#include <execinfo.h>

#include "handlers/MediaServiceHandler.h"
#include "handlers/MediaSessionServiceHandler.h"
#include "handlers/NetworkConnectionServiceHandler.h"
#include "handlers/MixerServiceHandler.h"

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <server/TThreadedServer.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include "media_config_loader.h"

#include <glibmm.h>
#include <fstream>

#include <kms-core.h>

#include "log.h"

#include <version.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::com::kurento::kms::api;

using boost::shared_ptr;
using com::kurento::kms::MediaServerServiceHandler;
using com::kurento::kms::MediaSessionServiceHandler;
using com::kurento::kms::NetworkConnectionServiceHandler;
using com::kurento::kms::MixerServiceHandler;
using com::kurento::kms::api::ServerConfig;
using ::com::kurento::log::Log;
using ::Glib::KeyFile;
using ::Glib::KeyFileFlags;

static Log l("main");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

#define DEFAULT_CONFIG_FILE "kms.conf"

#define SERVER_ADDRESS "localhost"
#define SERVER_SERVICE_PORT 9090
#define SESSION_SERVICE_PORT 9091
#define NETWORK_CONNECTION_SERVICE_PORT 9092
#define MIXER_SERVICE_PORT 9093

#define SERVER_ADDRESS_KEY "serverAddress"
#define SERVER_PORT_KEY "serverPort"
#define SESSION_PORT_KEY "sessionPort"
#define NETWORK_CONNECTION_PORT_KEY "connectionPort"
#define MIXER_PORT_KEY "mixerPort"

static ServerConfig config;
static SessionSpec sessionSpec;

static KeyFile configFile;

static void create_server_service() {
	int port;

	port = config.serverServicePort;

	shared_ptr<MediaServerServiceHandler> handler(
			new MediaServerServiceHandler(config, sessionSpec));
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

static void create_network_connection_service() {
	int port;

	if (!config.__isset.networkConnectionServicePort) {
		w("No port set in configuration for NetworkConnectionService");
		throw Glib::Thread::Exit();
	} else {
		port = config.networkConnectionServicePort;
	}

	shared_ptr<NetworkConnectionServiceHandler> handler(new NetworkConnectionServiceHandler());
	shared_ptr<TProcessor> processor(new NetworkConnectionServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting NetworkConnectionService");
	server->serve();

	i("NetworkConnectionService stopped finishing thread");
	throw Glib::Thread::Exit();
}

static void create_mixer_service() {
	int port;

	if (!config.__isset.mixerServicePort) {
		w("No port set in configuration for MixerService");
		throw Glib::Thread::Exit();
	} else {
		port = config.mixerServicePort;
	}

	shared_ptr<MixerServiceHandler> handler(new MixerServiceHandler());
	shared_ptr<TProcessor> processor(new MixerServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting MixerService");
	server->serve();

	i("MixerService stopped finishing thread");
	throw Glib::Thread::Exit();
}

static void set_default_server_config() {
	config.__set_address(SERVER_ADDRESS);
	config.__set_serverServicePort(SERVER_SERVICE_PORT);
	config.__set_mediaSessionServicePort(SESSION_SERVICE_PORT);
	config.__set_networkConnectionServicePort(NETWORK_CONNECTION_SERVICE_PORT);
	config.__set_mixerServicePort(MIXER_SERVICE_PORT);
}

static void check_port(int port) {
	if (port <=0 || port > G_MAXUSHORT)
		throw Glib::KeyFileError(Glib::KeyFileError::PARSE,
							"Invalid value");
}

static void load_config(const std::string &file_name) {
	gint port;

	i("Reading configuration from: " + file_name);
	/* Try to open de file */
	{
		std::ifstream file(file_name);
		if (!file) {
			i("Config file not found, creating a new one");
			std::ofstream of(file_name);
		}
	}
	try {
		if (!configFile.load_from_file(file_name,
				KeyFileFlags::KEY_FILE_KEEP_COMMENTS |
				KeyFileFlags::KEY_FILE_KEEP_TRANSLATIONS )) {
			w("Error loading configuration from " + file_name +
						", loading "
						"default server config, but no "
						"codecs will be available");
			set_default_server_config();
			return;
		}
	} catch (Glib::Error ex) {
		w("Error loading configuration: " + ex.what());
		w("Error loading configuration from " + file_name + ", loading "
						"default server config, but no "
						"codecs will be available");
		set_default_server_config();
		return;
	}

	try {
		config.__set_address(configFile.get_string(SERVER_GROUP,
							SERVER_ADDRESS_KEY));
	} catch (Glib::KeyFileError err) {
		i(err.what());
		i("Setting default address");
		configFile.set_string(SERVER_GROUP, SERVER_ADDRESS_KEY,
								SERVER_ADDRESS);
		config.__set_address(SERVER_ADDRESS);
	}

	try {
		port = configFile.get_integer(SERVER_GROUP, SERVER_PORT_KEY);
		check_port(port);
		config.__set_serverServicePort(port);
	} catch (Glib::KeyFileError err) {
		i(err.what());
		i("Setting default server port");
		configFile.set_integer(SERVER_GROUP, SERVER_PORT_KEY,
							SERVER_SERVICE_PORT);
		config.__set_serverServicePort(SERVER_SERVICE_PORT);
	}

	try {
		port = configFile.get_integer(SERVER_GROUP, SESSION_PORT_KEY);
		check_port(port);
		config.__set_mediaSessionServicePort(port);
	} catch (Glib::KeyFileError err) {
		i(err.what());
		i("Setting default media session port");
		configFile.set_integer(SERVER_GROUP, SESSION_PORT_KEY,
							SESSION_SERVICE_PORT);
		config.__set_mediaSessionServicePort(SESSION_SERVICE_PORT);
	}

	try {
		port = configFile.get_integer(SERVER_GROUP,
						NETWORK_CONNECTION_PORT_KEY);
		check_port(port);
		config.__set_networkConnectionServicePort(port);
	} catch (Glib::KeyFileError err) {
		i(err.what());
		i("Setting default network connection port");
		configFile.set_integer(SERVER_GROUP,
					NETWORK_CONNECTION_PORT_KEY,
					NETWORK_CONNECTION_SERVICE_PORT);
		config.__set_networkConnectionServicePort(
					NETWORK_CONNECTION_SERVICE_PORT);
	}

	try {
		port =configFile.get_integer(SERVER_GROUP, MIXER_PORT_KEY);
		check_port(port);
		config.__set_mixerServicePort(port);
	} catch (Glib::KeyFileError err) {
		i(err.what());
		i("Setting default mixer port");
		configFile.set_integer(SERVER_GROUP, MIXER_PORT_KEY,
							MIXER_SERVICE_PORT);
		config.__set_mixerServicePort(MIXER_SERVICE_PORT);
	}

	try {
		load_spec(configFile, sessionSpec);
		d("configured %d medias", sessionSpec.medias.size());
	} catch (Glib::KeyFileError err) {
		w(err.what());
		w("Wrong codec configuration, communication won't be possible");
	}

	std::ofstream f(file_name, std::ios::out | std::ios::trunc);
	f << configFile.to_data();
	f.close();

	i("Configuration loaded successfully");
	d("Final config file:\n" + configFile.to_data());
}

void
bt_sighandler(int sig, struct sigcontext ctx) {

	void *trace[16];
	char **messages = (char **)NULL;
	int i, trace_size = 0;

	if (sig == SIGSEGV)
		printf("Got signal %d, faulty address is %p, "
		"from %p\n", sig, (gpointer) ctx.cr2, (gpointer) ctx.eip);
	else
		printf("Got signal %d\n", sig);

	trace_size = backtrace(trace, 16);
	/* overwrite sigaction with caller's address */
	//trace[1] = (void *)ctx.eip;
	messages = backtrace_symbols(trace, trace_size);
	/* skip first stack frame (points here) */
	printf("[bt] Execution path:\n");
	for (i=1; i<trace_size; ++i) {
		printf("[bt] #%d %s\n", i, messages[i]);

		char syscom[256];
		//last parameter is the name of this app
		sprintf(syscom,"addr2line %p -e kmsc", trace[i]);
		system(syscom);
	}

	if (sig == SIGPIPE) {
		d("Ignore sigpipe");
	} else {
		exit(sig);
	}
}

int main(int argc, char **argv) {

	/* Install our signal handler */
	struct sigaction sa;

	sa.sa_handler = (void (*)(int))bt_sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);

	kms_init(&argc, &argv);

	Glib::thread_init();

	i("Kmsc version: %s", get_version());

	load_config(DEFAULT_CONFIG_FILE);

	sigc::slot<void> ss = sigc::ptr_fun(&create_server_service);
	Glib::Thread *serverServiceThread = Glib::Thread::create(ss, true);

	sigc::slot<void> mss = sigc::ptr_fun(&create_session_service);
	Glib::Thread::create(mss, true);

	sigc::slot<void> ncss = sigc::ptr_fun(&create_network_connection_service);
	Glib::Thread::create(ncss, true);

	sigc::slot<void> mxss = sigc::ptr_fun(&create_mixer_service);
	Glib::Thread::create(mxss, true);

	Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create(true);
	loop->run();
	serverServiceThread->join();

	// TODO: Finish all other threads and notify error

	return 0;
}
