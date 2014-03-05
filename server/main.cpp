/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include <signal.h>
#include <execinfo.h>
#include <config.h>

#include "MediaServerServiceHandler.hpp"

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <server/TNonblockingServer.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include "media_config.hpp"

#include <glibmm.h>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <version.hpp>
#include "httpendpointserver.hpp"
#include <ModuleLoader.hpp>

#define GST_CAT_DEFAULT kurento_media_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaServer"

#define ENV_VAR "GST_PLUGIN_PATH"

Glib::Mutex mutex;
Glib::Cond cond;
bool finish = FALSE;

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::kurento;

using boost::shared_ptr;
using namespace boost::filesystem;
using ::kurento::MediaServerServiceHandler;
using ::Glib::KeyFile;
using ::Glib::KeyFileFlags;

static std::string serverAddress, httpEPServerAddress,
       httpEPServerAnnouncedAddress;
static gint serverServicePort, httpEPServerServicePort;
GstSDPMessage *sdpPattern;
KmsHttpEPServer *httpepserver;
std::string stunServerAddress, pemCertificate;
gint stunServerPort;

Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create (true);
static TNonblockingServer *p_server = NULL;

std::map <std::string, KurentoModule *> modules;

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
  shared_ptr < MediaServerServiceHandler >
  handler (new MediaServerServiceHandler (modules) );
  shared_ptr < TProcessor >
  processor (new KmsMediaServerServiceProcessor (handler) );
  shared_ptr < TProtocolFactory >
  protocolFactory (new TBinaryProtocolFactory () );
  shared_ptr < PosixThreadFactory > threadFactory (new PosixThreadFactory () );
  shared_ptr < ThreadManager > threadManager =
    ThreadManager::newSimpleThreadManager (15);
  threadManager->threadFactory (threadFactory);
  threadManager->start ();
  TNonblockingServer server (processor, protocolFactory, serverServicePort,
                             threadManager);
  p_server = &server;
  GST_INFO ("Starting MediaServerService");
  kill (getppid(), SIGCONT);
  server.serve ();
  GST_INFO ("MediaServerService stopped finishing thread");
  throw Glib::Thread::Exit ();
}

static void
check_port (int port)
{
  if (port <= 0 || port > G_MAXUSHORT) {
    throw Glib::KeyFileError (Glib::KeyFileError::PARSE, "Invalid value");
  }
}

static void
set_default_media_server_config ()
{
  GST_WARNING ("Setting default configuration for media server. "
               "Using IP address: %s, port: %d. "
               "No codecs support will be available with default configuration.",
               MEDIA_SERVER_ADDRESS, MEDIA_SERVER_SERVICE_PORT);

  serverAddress = MEDIA_SERVER_ADDRESS;
  serverServicePort = MEDIA_SERVER_SERVICE_PORT;
}

static void
set_default_http_ep_server_config ()
{
  httpEPServerServicePort = HTTP_EP_SERVER_SERVICE_PORT;
  GST_WARNING ("Setting default configuration for http end point server. "
               "Using IP address: %s, port: %d. ",
               httpEPServerAddress.c_str (), httpEPServerServicePort);
}

static void
set_default_config ()
{
  set_default_media_server_config ();
  set_default_http_ep_server_config();
}

static gchar *
read_entire_file (const gchar *file_name)
{
  gchar *data;
  long f_size;
  FILE *fp;

  fp = fopen (file_name, "r");

  if (fp == NULL) {
    return NULL;
  }

  fseek (fp, 0, SEEK_END);
  f_size = ftell (fp);
  fseek (fp, 0, SEEK_SET);
  data = (gchar *) g_malloc0 (f_size + 1);

  if (fread (data, 1, f_size, fp) != (size_t) f_size) {
    GST_ERROR ("Error reading file");
  }

  fclose (fp);

  data[f_size] = '\0';

  return data;
}

static GstSDPMessage *
load_sdp_pattern (Glib::KeyFile &configFile, const std::string &confFileName)
{
  GstSDPResult result;
  GstSDPMessage *sdp_pattern = NULL;
  gchar *sdp_pattern_text;
  std::string sdp_pattern_file_name;

  GST_DEBUG ("Load SDP Pattern");
  result = gst_sdp_message_new (&sdp_pattern);

  if (result != GST_SDP_OK) {
    GST_ERROR ("Error creating sdp message");
    return NULL;
  }

  sdp_pattern_file_name = configFile.get_string (SERVER_GROUP, SDP_PATTERN_KEY);
  boost::filesystem::path p (confFileName.c_str () );
  sdp_pattern_file_name.insert (0, "/");
  sdp_pattern_file_name.insert (0, p.parent_path ().c_str() );
  sdp_pattern_text = read_entire_file (sdp_pattern_file_name.c_str () );

  if (sdp_pattern_text == NULL) {
    GST_ERROR ("Error reading SDP pattern file");
    gst_sdp_message_free (sdp_pattern);
    return NULL;
  }

  result = gst_sdp_message_parse_buffer ( (const guint8 *) sdp_pattern_text, -1,
                                          sdp_pattern);
  g_free (sdp_pattern_text);

  if (result != GST_SDP_OK) {
    GST_ERROR ("Error parsing SDP config pattern");
    gst_sdp_message_free (sdp_pattern);
    return NULL;
  }

  return sdp_pattern;
}

static void
configure_kurento_media_server (KeyFile &configFile,
                                const std::string &file_name)
{
  gint port;
  gchar *sdpMessageText = NULL;

  try {
    serverAddress = configFile.get_string (SERVER_GROUP,
                                           MEDIA_SERVER_ADDRESS_KEY);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Setting default address %s to media server",
                 MEDIA_SERVER_ADDRESS);
    serverAddress = MEDIA_SERVER_ADDRESS;
  }

  try {
    port = configFile.get_integer (SERVER_GROUP, MEDIA_SERVER_SERVICE_PORT_KEY);
    check_port (port);
    serverServicePort = port;
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Setting default port %d to media server",
                 MEDIA_SERVER_SERVICE_PORT);
    serverServicePort = MEDIA_SERVER_SERVICE_PORT;
  }

  try {
    sdpPattern = load_sdp_pattern (configFile, file_name);
    GST_DEBUG ("SDP: \n%s", sdpMessageText = gst_sdp_message_as_text (sdpPattern) );
    g_free (sdpMessageText);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Wrong codec configuration, communication won't be possible");
  }
}

static void
configure_http_ep_server (KeyFile &configFile)
{
  gint port;

  try {
    httpEPServerAddress = configFile.get_string (HTTP_EP_SERVER_GROUP,
                          HTTP_EP_SERVER_ADDRESS_KEY);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Http end point server will be listening to all interfaces");
  }

  try {
    port = configFile.get_integer (HTTP_EP_SERVER_GROUP,
                                   HTTP_EP_SERVER_SERVICE_PORT_KEY);
    check_port (port);
    httpEPServerServicePort = port;
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Setting default port %d to http end point server",
                 HTTP_EP_SERVER_SERVICE_PORT);
    httpEPServerServicePort = HTTP_EP_SERVER_SERVICE_PORT;
  }

  try {
    httpEPServerAnnouncedAddress = configFile.get_string (HTTP_EP_SERVER_GROUP,
                                   HTTP_EP_SERVER_ANNOUNCED_ADDRESS_KEY);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Http end point server will choose any available "
                 "IP address to compose URLs");
  }
}

static void
configure_web_rtc_end_point (KeyFile &configFile, const std::string &file_name)
{
  gint port;
  std::string pem_certificate_file_name;

  try {
    stunServerAddress = configFile.get_string (WEB_RTC_END_POINT_GROUP,
                        WEB_RTC_END_POINT_STUN_SERVER_ADDRESS_KEY);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Setting default address %s to stun server",
                 STUN_SERVER_ADDRESS);
    stunServerAddress = STUN_SERVER_ADDRESS;
  }

  try {
    port = configFile.get_integer (WEB_RTC_END_POINT_GROUP,
                                   WEB_RTC_END_POINT_STUN_SERVER_PORT_KEY);
    check_port (port);
    stunServerPort = port;
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Setting default port %d to stun server",
                 STUN_SERVER_PORT);
    stunServerPort = STUN_SERVER_PORT;
  }

  try {
    pem_certificate_file_name = configFile.get_string (WEB_RTC_END_POINT_GROUP,
                                WEB_RTC_END_POINT_PEM_CERTIFICATE_KEY);
    boost::filesystem::path p (file_name.c_str () );
    pem_certificate_file_name.insert (0, "/");
    pem_certificate_file_name.insert (0, p.parent_path ().c_str() );
    pemCertificate = pem_certificate_file_name.c_str();

  } catch (const Glib::KeyFileError &err) {
    pemCertificate = "";
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Pem Certificate not found.");
  }
}

static void
load_config (const std::string &file_name)
{
  KeyFile configFile;

  GST_INFO ("Reading configuration from: %s", file_name.c_str () );

  /* Try to load configuration file */
  try {
    if (!configFile.load_from_file (file_name,
                                    KeyFileFlags::KEY_FILE_KEEP_COMMENTS |
                                    KeyFileFlags::KEY_FILE_KEEP_TRANSLATIONS) ) {
      GST_WARNING ("Can not load configuration file %s", file_name.c_str () );
      set_default_config ();
      return;
    }
  } catch (const Glib::Error &ex) {
    GST_ERROR ("Error loading configuration: %s", ex.what ().c_str () );
    set_default_config ();
    return;
  }

  /* parse options so as to configure servers */
  configure_kurento_media_server (configFile, file_name);
  configure_http_ep_server (configFile);
  configure_web_rtc_end_point (configFile, file_name);

  GST_INFO ("Configuration loaded successfully");
}

static void
initialiseExecutableName (char *exe, int size)
{
  char link[1024];
  int len;
  snprintf (link, sizeof (link), "/proc/%d/exe", getpid () );
  len = readlink (link, exe, size);

  if (len == -1) {
    g_printerr ("ERROR GETTING NAME\n");
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

static bool
quit_loop ()
{
  loop->quit ();

  return FALSE;
}

static void
bt_sighandler (int sig, siginfo_t *info, gpointer data)
{
  void *trace[35];
  char **messages = (char **) NULL;
  int i, trace_size = 0;

  g_print ("Got signal %d\n", sig);

  /* Do something useful with siginfo_t */
  if (sig == SIGSEGV) {
    g_printerr ("Got signal %d, faulty address is %p\n", sig,
                (gpointer) info->si_addr);
  } else if (sig == SIGKILL || sig == SIGINT) {
    /* since we connect to a signal handler, asynchronous management might */
    /* might happen so we need to set an idle handler to exit the main loop */
    /* in the mainloop context. */
    Glib::RefPtr<Glib::IdleSource> idle_source = Glib::IdleSource::create ();
    idle_source->connect (sigc::ptr_fun (&quit_loop) );
    idle_source->attach (loop->get_context() );
    return;
  }

  trace_size = backtrace (trace, 35);
  /* overwrite sigaction with caller's address */
  messages = backtrace_symbols (trace, trace_size);
  /* skip first stack frame (points here) */
  g_print ("\t[bt] Execution path:\n");

  for (i = 1; i < trace_size; ++i) {
    g_print ("\t[bt] #%d %s\n", i, messages[i]);
    char syscom[256];
    gchar **strs;
    const gchar *exe;
    strs = g_strsplit (messages[i], "(", 2);

    if (strs[1] == NULL) {
      exe = getExecutableName ();
    } else {
      exe = strs[0];
    }

    sprintf (syscom, "echo -n \"\t[bt]\t\t\"; addr2line %p -s -e %s",
             trace[i], exe);
    g_strfreev (strs);

    if (system (syscom) == -1) {
      g_printerr ("Error calling addr2line\n");
    }
  }

  if (sig == SIGPIPE) {
    GST_DEBUG ("Ignore sigpipe");
  } else {
    exit (sig);
  }
}

static void
http_server_start_cb (KmsHttpEPServer *self, GError *err, gpointer user_data)
{
  if (err != NULL) {
    GST_ERROR ("Http server could not start. Reason: %s", err->message);
    loop->quit ();
    return;
  }

  GST_DEBUG ("HttpEPServer started. Setting up thrift server service.");

  /* Created thread not used for joining because of a bug in thrift */
  sigc::slot < void >ss = sigc::ptr_fun (&create_media_server_service);
  Glib::Thread::create (ss, true);
}

static void
http_server_stop_cb (KmsHttpEPServer *self, GError *err, gpointer user_data)
{
  if (err != NULL) {
    GST_ERROR ("Http server could not be stopped. Reason: %s", err->message);
  }

  mutex.lock();
  finish = TRUE;
  cond.signal();
  mutex.unlock();
}

static void
check_if_plugins_are_available ()
{
  GstPlugin *plugin = gst_plugin_load_by_name ("kurento");

  if (plugin == NULL) {
    g_printerr ("Kurento plugin not found, try adding the plugins route with "
                "--gst-plugin-path parameter. See help (--help-gst) "
                "for more info\n");
    exit (1);
  }

  g_clear_object (&plugin);
}

int
main (int argc, char **argv)
{
  GError *error = NULL;
  GOptionContext *context;
  struct sigaction sa;
  gchar *oldEnv, *newEnv;

  oldEnv = getenv (ENV_VAR);

  if (oldEnv == NULL) {
    newEnv = g_strdup_printf ("%s", PLUGIN_PATH);
  } else {
    newEnv = g_strdup_printf ("%s:%s", oldEnv, PLUGIN_PATH);
  }

  setenv (ENV_VAR, newEnv, 1);
  g_free (newEnv);

  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gst_init_get_option_group () );

  if (!g_option_context_parse (context, &argc, &argv, &error) ) {
    g_printerr ("option parsing failed: %s\n", error->message);
    g_error_free (error);
    exit (1);
  }

  g_option_context_free (context);

  gst_init (&argc, &argv);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_if_plugins_are_available ();

  /* Install our signal handler */
  sa.sa_sigaction = bt_sighandler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_SIGINFO;
  sigaction (SIGSEGV, &sa, NULL);
  sigaction (SIGPIPE, &sa, NULL);
  sigaction (SIGINT, &sa, NULL);
  sigaction (SIGKILL, &sa, NULL);
  Glib::thread_init ();
  GST_INFO ("Kmsc version: %s", get_version () );

  if (!conf_file) {
    load_config (DEFAULT_CONFIG_FILE);
  } else {
    load_config ( (std::string) conf_file);
  }

  modules = load_modules();

  /* Start Http End Point Server */
  GST_DEBUG ("Starting Http end point server.");
  httpepserver = kms_http_ep_server_new (
                   KMS_HTTP_EP_SERVER_PORT, httpEPServerServicePort,
                   KMS_HTTP_EP_SERVER_INTERFACE,
                   (httpEPServerAddress.empty() ) ? NULL : httpEPServerAddress.c_str (),
                   KMS_HTTP_EP_SERVER_ANNOUNCED_IP,
                   (httpEPServerAnnouncedAddress.empty() ) ? NULL :
                   httpEPServerAnnouncedAddress.c_str (),
                   NULL);

  kms_http_ep_server_start (httpepserver, http_server_start_cb, NULL, NULL);

  loop->run ();

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver, http_server_stop_cb, NULL, NULL);

  /* TODO: Improve this to with a more sophisticated method to start
   * and stop services. */
  /* Wait until server has stopped */
  mutex.lock();

  while (!finish) {
    cond.wait (mutex);
  }

  mutex.unlock();

  g_object_unref (G_OBJECT (httpepserver) );

  return 0;
}
