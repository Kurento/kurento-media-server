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
#include "modules.hpp"

#define GST_CAT_DEFAULT kurento_media_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaServer"

const std::string DEFAULT_CONFIG_FILE = "/etc/kurento/kurento.conf.json";
const std::string ENV_PREFIX = "KURENTO_";

using namespace ::kurento;

Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create ();

static std::shared_ptr<Transport>
load_config (boost::property_tree::ptree &config, const std::string &file_name)
{
  std::shared_ptr<Transport> transport;
  boost::filesystem::path configFilePath (file_name);
  GST_INFO ("Reading configuration from: %s", file_name.c_str () );

  try {
    boost::property_tree::read_json (file_name, config);
  } catch (boost::property_tree::ptree_error &e) {
    GST_ERROR ("Error reading configuration: %s", e.what() );
    std::cerr << "Error reading configuration: " << e.what() << std::endl;
    exit (1);
  }

  config.add ("configPath", configFilePath.parent_path().string() );

  GST_INFO ("Configuration loaded successfully");

  std::shared_ptr<ServerMethods> serverMethods (new ServerMethods (config) );

  try {
    transport = TransportFactory::create_transport (config, serverMethods);
  } catch (std::exception &e) {
    GST_ERROR ("Error creating transport: %s", e.what() );
    exit (1);
  }

  return transport;
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

int
main (int argc, char **argv)
{
  sigset_t mask;
  std::shared_ptr <SignalHandler> signalHandler;
  std::shared_ptr<Transport> transport;
  boost::property_tree::ptree config;
  std::string confFile;
  std::string path;

  Glib::init();

  gst_init (&argc, &argv);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  try {
    boost::program_options::options_description desc ("kurento-media-server usage");

    desc.add_options()
    ("help,h", "Display this help message")
    ("version,v", "Display the version number")
    ("log,g", "Use gstreamer log")
    ("list,l", "Lists all available factories")
    ("modules-path,p", boost::program_options::value<std::string>
     (&path), "Path where kurento modules can be found")
    ("conf-file,f", boost::program_options::value<std::string>
     (&confFile)->default_value (DEFAULT_CONFIG_FILE),
     "Configuration file location");

    boost::program_options::command_line_parser clp (argc, argv);
    clp.options (desc).allow_unregistered();
    boost::program_options::variables_map vm;
    auto parsed = clp.run();
    boost::program_options::store (parsed, vm);

    boost::program_options::store (boost::program_options::parse_environment (desc,
    [&desc] (std::string & input) -> std::string {
      /* Look for KURENTO_ prefix and change to lower case */
      if (input.find (ENV_PREFIX) == 0)
      {
        std::string aux = input.substr (ENV_PREFIX.size() );
        std::transform (aux.begin(), aux.end(), aux.begin(), [] (int c) -> int {
          return (c == '_') ? '-' : tolower (c);
        });

        if (!desc.find_nothrow (aux, false) ) {
          return "";
        }

        return aux;
      }


      return "";
    }), vm);

    boost::program_options::notify (vm);

    if (!vm.count ("log") ) {
      gst_debug_remove_log_function_by_data (NULL);
      gst_debug_add_log_function (simple_log_function, NULL, NULL);
    }

    if (vm.count ("help") ) {
      std::cout << desc << "\n";
      exit (0);
    }

    if (vm.count ("version") || vm.count ("list") ) {
      // Disable log to just print version
      gst_debug_remove_log_function_by_data (NULL);
    }

    loadModules (path);

    if (vm.count ("list") ) {
      std::cout << "Available factories:" << std::endl;

      for (auto it : kurento::getModuleManager().getLoadedFactories() ) {
        std::cout << "\t" << it.first << std::endl;
      }

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

  transport = load_config (config, confFile);

  /* Start transport */
  transport->start ();

  GST_INFO ("Mediaserver started");

  loop->run ();

  transport->stop();

  GST_INFO ("Mediaserver stopped");

  return 0;
}
