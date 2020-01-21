/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <config.h>

#include "death_handler.hpp"

#include <glibmm.h>
#include <fstream>
#include <iostream>
#include "version.hpp"
#include <glib/gstdio.h>
#include <ftw.h>

#include <boost/log/utility/setup/common_attributes.hpp>

#include "TransportFactory.hpp"
#include "ResourceManager.hpp"

#include <ServerMethods.hpp>
#include <gst/gst.h>

#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/optional.hpp>

#include "logging.hpp"
#include "modules.hpp"
#include "loadConfig.hpp"

#include "MediaSet.hpp"

#define GST_CAT_DEFAULT kurento_media_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaServer"

const std::string DEFAULT_CONFIG_FILE = "/etc/kurento/kurento.conf.json";
const std::string ENV_PREFIX = "KURENTO_";
const int DEFAULT_LOG_FILE_SIZE = 100;
const int DEFAULT_LOG_FILE_COUNT = 10;

using namespace ::kurento;
namespace logging = boost::log;

Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create ();

static std::shared_ptr<Transport>
createTransportFromConfig (boost::property_tree::ptree &config)
{
  std::shared_ptr<ServerMethods> serverMethods (new ServerMethods (config) );
  std::shared_ptr<Transport> transport;

  try {
    transport = TransportFactory::create_transport (config, serverMethods);
  } catch (std::exception &e) {
    GST_ERROR ("Error creating transport: %s", e.what() );
    exit (1);
  }

  return transport;
}

static void
signal_handler (int signo)
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

  default:
    break;
  }
}

static void
kms_init_dependencies (int *argc, char ***argv)
{
  Glib::init();

  gst_init (argc, argv);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  /* Initialization routine to add commonly used */
  /* attributes to the logging system */
  logging::add_common_attributes();
}

int
main (int argc, char **argv)
{
  struct sigaction signalAction {};
  std::shared_ptr<Transport> transport;
  boost::property_tree::ptree config;
  std::string confFile;
  std::string modulesPath, logsPath, modulesConfigPath;
  int fileSize, fileNumber;

  Debug::DeathHandler dh;
  dh.set_thread_safe (true);

  kms_init_dependencies (&argc, &argv);

  if (gst_debug_get_color_mode() == GST_DEBUG_COLOR_MODE_OFF) {
    dh.set_color_output (false);
  } else {
    dh.set_color_output (true);
  }

  try {
    boost::program_options::options_description desc ("kurento-media-server usage");

    desc.add_options()
    ("help,h", "Display this help message")
    ("version,v", "Display the version number")
    ("list,l", "Lists all available factories")
    ("modules-path,p", boost::program_options::value<std::string>
     (&modulesPath), "Colon-separated path(s) where Kurento modules can be found")
    ("conf-file,f", boost::program_options::value<std::string>
     (&confFile)->default_value (DEFAULT_CONFIG_FILE),
     "Configuration file location")
    ("logs-path,d", boost::program_options::value <std::string> (&logsPath),
     "Path where rotating log files will be stored")
    ("modules-config-path,c",
     boost::program_options::value <std::string> (&modulesConfigPath),
     "Path where modules config files can be found")
    ("log-file-size,s",
     boost::program_options::value <int> (&fileSize)->default_value (
       DEFAULT_LOG_FILE_SIZE),
     "Maximum file size for log files, in MB")
    ("number-log-files,n",
     boost::program_options::value <int> (&fileNumber)->default_value (
       DEFAULT_LOG_FILE_COUNT),
     "Maximum number of log files to keep");

    boost::program_options::command_line_parser clp (argc, argv);
    clp.options (desc).allow_unregistered();
    boost::program_options::variables_map vm;
    auto parsed = clp.run();
    boost::program_options::store (parsed, vm);

    boost::program_options::store (boost::program_options::parse_environment (desc,
    [&desc] (std::string input) -> std::string {
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

    kms_init_logging ();

    if (vm.count ("logs-path") ) {
      if (kms_init_logging_files (logsPath, fileSize, fileNumber) ) {
        GST_INFO ("Logs storage path set to %s", logsPath.c_str() );
      } else {
        GST_WARNING ("Cannot set logs storage path to %s", logsPath.c_str() );
      }
    }

    if (vm.count ("help") ) {
      std::cout << desc << "\n";
      exit (0);
    }

    if (vm.count ("version") || vm.count ("list") ) {
      // Disable log to just print version
      gst_debug_remove_log_function_by_data(nullptr);
    }

    loadModules (modulesPath);

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
  signalAction.sa_handler = signal_handler;

  sigaction(SIGINT, &signalAction, nullptr);
  sigaction(SIGTERM, &signalAction, nullptr);
  sigaction(SIGPIPE, &signalAction, nullptr);

  GST_INFO ("Kurento Media Server version: %s", get_version () );

  loadConfig (config, confFile, modulesConfigPath);

  boost::optional<float> killResourceLimit =
    config.get_optional<float> ("mediaServer.resources.killLimit");

  if (killResourceLimit) {
    GST_INFO ("Using above %.2f%% of system limits will kill the server when no objects are alive",
              *killResourceLimit * 100.0f);

    killServerOnLowResources (*killResourceLimit);
  }

  transport = createTransportFromConfig (config);

  /* Start transport */
  transport->start ();

  GST_INFO ("Kurento Media Server started");

  loop->run ();

  transport->stop();
  MediaSet::deleteMediaSet();

  GST_INFO ("Kurento Media Server stopped");

  return 0;
}
