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

#include <config.h>

#include <glibmm.h>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include "version.hpp"
#include <glib/gstdio.h>
#include <ftw.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "TransportFactory.hpp"

#include <SignalHandler.hpp>
#include <ServerMethods.hpp>
#include <gst/gst.h>

#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include "logging.hpp"

#define GST_CAT_DEFAULT kurento_media_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaServer"

const std::string DEFAULT_CONFIG_FILE = "/etc/kurento/kurento.conf.json";
const std::string ENV_PREFIX = "KURENTO_";

using namespace ::kurento;

static std::shared_ptr<Transport> transport;

__pid_t pid;

Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create ();

static gchar *tmp_dir;

Glib::RefPtr<Glib::IOChannel> channel;

static void
load_config (boost::property_tree::ptree &config, const std::string &file_name)
{
  boost::filesystem::path configFilePath (file_name);
  GST_INFO ("Reading configuration from: %s", file_name.c_str () );

  boost::property_tree::read_json (file_name, config);

  config.add ("configPath", configFilePath.parent_path().string() );
  pid = getpid();

  GST_INFO ("Configuration loaded successfully");

  std::shared_ptr<ServerMethods> serverMethods (new ServerMethods (config) );

  try {
    transport = TransportFactory::create_transport (config, serverMethods);
  } catch (std::exception &e) {
    GST_ERROR ("Error creating transport: %s", e.what() );
    exit (1);
  }
}

static void
signal_handler (uint32_t signo)
{
  static unsigned int __terminated = 0;

  switch (signo) {
  case SIGINT:
  case SIGTERM:
    if (__terminated == 0) {
      GST_DEBUG ("Terminating.");
      loop->quit ();
    }

    __terminated = 1;
    break;

  case SIGPIPE:
    GST_DEBUG ("Ignore sigpipe signal");
    break;

  case SIGSEGV:
    GST_DEBUG ("Segmentation fault. Aborting process execution");
    abort ();

  default:
    break;
  }
}

static int
delete_file (const char *fpath, const struct stat *sb, int typeflag,
             struct FTW *ftwbuf)
{
  int rv = g_remove (fpath);

  if (rv) {
    GST_WARNING ("Error deleting file: %s. %s", fpath, strerror (errno) );
  }

  return rv;
}
static void
remove_recursive (const gchar *path)
{
  nftw (path, delete_file, 64, FTW_DEPTH | FTW_PHYS);
}

static void
deleteCertificate ()
{
  // Only parent process can delete certificate
  if (pid != getpid() ) {
    return;
  }

  if (tmp_dir != NULL) {
    remove_recursive (tmp_dir);
    g_free (tmp_dir);
  }
}

static std::string
environment_adaptor (std::string &input)
{
  /* Look for KMS_ prefix and change to lower case */
  if (input.find (ENV_PREFIX) == 0) {
    std::string aux = input.substr (ENV_PREFIX.size() );
    std::transform (aux.begin(), aux.end(), aux.begin(), [] (int c) -> int {
      return (c == '_') ? '-' : tolower (c);
    });
    return aux;
  }

  return "";
}

int
main (int argc, char **argv)
{
  sigset_t mask;
  std::shared_ptr <SignalHandler> signalHandler;
  boost::property_tree::ptree config;
  std::string confFile;

  Glib::init();

  gst_init (&argc, &argv);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
  gst_debug_remove_log_function_by_data (NULL);
  gst_debug_add_log_function (log_function, NULL, NULL);

  try {
    boost::program_options::options_description desc ("kurento-media-server usage");

    desc.add_options()
    ("help,h", "Display this help message")
    ("version,v", "Display the version number")
    ("conf-file,f", boost::program_options::value<std::string>
     (&confFile)->default_value (DEFAULT_CONFIG_FILE),
     "Configuration file location");

    boost::program_options::variables_map vm;
    boost::program_options::store (boost::program_options::parse_command_line (argc,
                                   argv, desc), vm);
    boost::program_options::store (boost::program_options::parse_environment (desc,
                                   &environment_adaptor), vm);
    boost::program_options::notify (vm);

    if (vm.count ("help") ) {
      std::cout << desc << "\n";
      exit (0);
    }

    if (vm.count ("version") ) {
      print_version();
      exit (0);
    }

  } catch (boost::program_options::error &e) {
    std::cerr <<  "Error : " << e.what() << std::endl;
    exit (1);
  }

  /* Install our signal handler */
  sigemptyset (&mask);
  sigaddset (&mask, SIGINT);
  sigaddset (&mask, SIGTERM);
  sigaddset (&mask, SIGSEGV);
  sigaddset (&mask, SIGPIPE);
  signalHandler = std::shared_ptr <SignalHandler> (new SignalHandler (mask,
                  signal_handler) );

  GST_INFO ("Kmsc version: %s", get_version () );

  load_config (config, confFile);

  /* Start transport */
  transport->start ();

  GST_INFO ("Mediaserver started");

  loop->run ();

  signalHandler.reset();

  deleteCertificate ();

  transport->stop();

  transport.reset();

  return 0;
}
