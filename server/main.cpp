/*
 * main.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <signal.h>
#include <execinfo.h>

#include "MediaServerServiceHandler.hpp"

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <server/TNonblockingServer.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include "media_config_loader.hpp"

#include <glibmm.h>
#include <fstream>

#include <gst/gst.h>

#include <version.hpp>
#include "log.hpp"

#define GST_DEFAULT_NAME "media_server"

GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::kurento;

using boost::shared_ptr;
using ::kurento::MediaServerServiceHandler;
using ::Glib::KeyFile;
using ::Glib::KeyFileFlags;

#define DEFAULT_CONFIG_FILE "/etc/kurento/kurento.conf"

#define MEDIA_SERVER_ADDRESS "localhost"
#define MEDIA_SERVER_SERVICE_PORT 9090

#define MEDIA_SERVER_ADDRESS_KEY "serverAddress"
#define MEDIA_SERVER_SERVICE_PORT_KEY "serverPort"

static std::string serverAddress;
static gint serverServicePort;
static GstSDPMessage *sdp_message;
static KeyFile configFile;

Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create (true);
static TNonblockingServer *p_server = NULL;

static gchar *conf_file;

static GOptionEntry entries[] = {
  {
    "conf-file", 'f', 0, G_OPTION_ARG_FILENAME, &conf_file, "Configuration file",
    NULL
  },
  {NULL}
};

static void
create_media_server_service ()
{
  int port;
  port = MEDIA_SERVER_SERVICE_PORT;
  shared_ptr < MediaServerServiceHandler >
  handler (new MediaServerServiceHandler () );
  shared_ptr < TProcessor >
  processor (new MediaServerServiceProcessor (handler) );
  shared_ptr < TProtocolFactory >
  protocolFactory (new TBinaryProtocolFactory () );
  shared_ptr < PosixThreadFactory > threadFactory (new PosixThreadFactory () );
  shared_ptr < ThreadManager > threadManager =
    ThreadManager::newSimpleThreadManager (15);
  threadManager->threadFactory (threadFactory);
  threadManager->start ();
  TNonblockingServer server (processor, protocolFactory, port, threadManager);
  p_server = &server;
  GST_INFO ("Starting MediaServerService");
  server.serve ();
  GST_INFO ("MediaServerService stopped finishing thread");
  throw Glib::Thread::Exit ();
}

static void
check_port (int port)
{
  if (port <= 0 || port > G_MAXUSHORT)
    throw Glib::KeyFileError (Glib::KeyFileError::PARSE, "Invalid value");
}

static void
set_default_server_config ()
{
  serverAddress = MEDIA_SERVER_ADDRESS;
  serverServicePort = MEDIA_SERVER_SERVICE_PORT;
}

static void
load_config (const std::string &file_name)
{
  gint port;
  GST_INFO ("Reading configuration from: %s", file_name.c_str () );
  /* Try to open de file */
  {
    std::ifstream file (file_name);

    if (!file) {
      GST_INFO ("Config file not found, creating a new one");
      std::ofstream of (file_name);
    }
  }

  try {
    if (!configFile.load_from_file (file_name,
        KeyFileFlags::KEY_FILE_KEEP_COMMENTS |
        KeyFileFlags::KEY_FILE_KEEP_TRANSLATIONS) ) {
      GST_WARNING ("Error loading configuration from %s, loading "
          "default server config, but no "
          "codecs will be available", file_name.c_str () );
      set_default_server_config ();
      return;
    }
  } catch (Glib::Error ex) {
    GST_WARNING ("Error loading configuration: %s", ex.what ().c_str () );
    GST_WARNING ("Error loading configuration from %s, loading "
        "default server config, but no "
        "codecs will be available", file_name.c_str () );
    set_default_server_config ();
    return;
  }

  try {
    serverAddress = configFile.get_string (SERVER_GROUP,
        MEDIA_SERVER_ADDRESS_KEY);
  } catch (Glib::KeyFileError err) {
    GST_INFO (err.what ().c_str () );
    GST_INFO ("Setting default address");
    configFile.set_string (SERVER_GROUP, MEDIA_SERVER_ADDRESS_KEY,
        MEDIA_SERVER_ADDRESS);
    serverAddress = MEDIA_SERVER_ADDRESS;
  }

  try {
    port = configFile.get_integer (SERVER_GROUP, MEDIA_SERVER_SERVICE_PORT_KEY);
    check_port (port);
    serverServicePort = port;
  } catch (Glib::KeyFileError err) {
    GST_INFO (err.what ().c_str () );
    GST_INFO ("Setting default server port");
    configFile.set_integer (SERVER_GROUP, MEDIA_SERVER_SERVICE_PORT_KEY,
        MEDIA_SERVER_SERVICE_PORT);
    serverServicePort = MEDIA_SERVER_SERVICE_PORT;
  }

  try {
    sdp_message = load_session_descriptor (configFile);
  } catch (Glib::KeyFileError err) {
    GST_WARNING (err.what ().c_str () );
    GST_WARNING ("Wrong codec configuration, communication won't be possible");
  }

  std::ofstream f (file_name, std::ios::out | std::ios::trunc);
  f << configFile.to_data ();
  f.close ();
  GST_INFO ("Configuration loaded successfully");
  GST_DEBUG ("Final config file:\n%s", configFile.to_data ().c_str () );
}

static void
initialiseExecutableName (char *exe, int size)
{
  char link[1024];
  int len;
  snprintf (link, sizeof (link), "/proc/%d/exe", getpid () );
  len = readlink (link, exe, size);

  if (len == -1) {
    fprintf (stderr, "ERROR GETTING NAME\n");
    exit (1);
  }

  exe[len] = '\0';
}

static const char *
getExecutableName ()
{
  static char *exe = NULL;
  static char aux[1024];

  if (exe == NULL) {
    initialiseExecutableName (aux, sizeof (aux) );
    exe = aux;
  }

  return exe;
}

static void
bt_sighandler (int sig, siginfo_t *info, gpointer data)
{
  void *trace[35];
  char **messages = (char **) NULL;
  int i, trace_size = 0;

//      ucontext_t *uc = (ucontext_t *)data;

  /* Do something useful with siginfo_t */
  if (sig == SIGSEGV) {
    printf ("Got signal %d, faulty address is %p\n", sig,
        (gpointer) info->si_addr);
  } else if (sig == SIGKILL || sig == SIGINT) {
    loop->quit ();

    if (p_server != NULL) {
      p_server->stop ();
      p_server = NULL;
    }

    return;
  } else {
    printf ("Got signal %d\n", sig);
  }

  trace_size = backtrace (trace, 35);
  /* overwrite sigaction with caller's address */
  //trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];
  messages = backtrace_symbols (trace, trace_size);
  /* skip first stack frame (points here) */
  g_print ("\t[bt] Execution path:\n");

  for (i = 1; i < trace_size; ++i) {
    g_print ("\t[bt] #%d %s\n", i, messages[i]);
    char syscom[256];
    gchar **strs;
    const gchar *exe;
    strs = g_strsplit (messages[i], "(", 2);

    if (strs[1] == NULL)
      exe = getExecutableName ();
    else
      exe = strs[0];

    sprintf (syscom, "echo -n \"\t[bt]\t\t\"; addr2line %p -s -e %s",
        trace[i], exe);
    g_strfreev (strs);
    system (syscom);
  }

  if (sig == SIGPIPE) {
    GST_DEBUG ("Ignore sigpipe");
  } else {
    exit (sig);
  }
}

int
main (int argc, char **argv)
{
  GError *error = NULL;
  GOptionContext *context;
  struct sigaction sa;
  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gst_init_get_option_group () );

  if (!g_option_context_parse (context, &argc, &argv, &error) ) {
    GST_ERROR ("option parsing failed: %s\n", error->message);
    exit (1);
  }

  gst_init (&argc, &argv);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
  /* Install our signal handler */
  sa.sa_sigaction = /*(void (*)(int, siginfo*, gpointer)) */ bt_sighandler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_SIGINFO;
  sigaction (SIGSEGV, &sa, NULL);
  sigaction (SIGPIPE, &sa, NULL);
  sigaction (SIGINT, &sa, NULL);
  sigaction (SIGKILL, &sa, NULL);
  Glib::thread_init ();
  GST_INFO ("Kmsc version: %s", get_version () );

  if (!conf_file)
    load_config (DEFAULT_CONFIG_FILE);
  else
    load_config ( (std::string) conf_file);

  sigc::slot < void >ss = sigc::ptr_fun (&create_media_server_service);
  Glib::Thread *serverServiceThread = Glib::Thread::create (ss, true);
  loop->run ();
  serverServiceThread->join ();
  return 0;
}
