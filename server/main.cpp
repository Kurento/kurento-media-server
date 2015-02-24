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

#include <queue>
#include <boost/foreach.hpp>

#include <glibmm.h>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include "version.hpp"
#include <glib/gstdio.h>
#include <ftw.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

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
namespace logging = boost::log;

Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create ();

static std::list<std::string>
split (const std::string &s, char delim)
{
  std::list<std::string> elems;
  std::stringstream ss (s);
  std::string item;

  while (std::getline (ss, item, delim) ) {
    elems.push_back (item);
  }

  return elems;
}

static void
mergePropertyTrees (boost::property_tree::ptree &ptMerged,
                    const boost::property_tree::ptree &rptSecond )
{
  // Keep track of keys and values (subtrees) in second property tree
  std::queue<std::string> qKeys;
  std::queue<boost::property_tree::ptree> qValues;
  qValues.push ( rptSecond );

  // Iterate over second property tree
  while ( !qValues.empty() ) {
    // Setup keys and corresponding values
    boost::property_tree::ptree ptree = qValues.front();
    qValues.pop();
    std::string keychain;

    if ( !qKeys.empty() ) {
      keychain = qKeys.front();
      qKeys.pop();
    }

    BOOST_FOREACH ( const boost::property_tree::ptree::value_type & child, ptree ) {
      if ( child.second.size() == 0 ) {
        std::string s;

        if ( !keychain.empty() ) {
          s = keychain + "." + child.first.data();
        } else {
          s = child.first.data();
        }

        // Put into combined property tree
        ptMerged.put ( s, child.second.data() );
      } else {
        // Put keys (identifiers of subtrees) and all of its parents (where present)
        // aside for later iteration.
        if ( !keychain.empty() ) {
          qKeys.push ( keychain + "." + child.first.data() );
        } else {
          qKeys.push ( child.first.data() );
        }

        // Put values (the subtrees) aside, too
        qValues.push ( child.second );
      }
    }
  }
}

static void
loadModulesConfigFromDir (boost::property_tree::ptree &config,
                          const boost::filesystem::path &dir, const boost::filesystem::path &parentDir)
{
  GST_INFO ("Looking for config files in %s", dir.string().c_str() );

  if (!boost::filesystem::is_directory (dir) ) {
    GST_WARNING ("Unable to load config files from: %s, it is not a directory",
                 dir.string().c_str() );
    return;
  }

  boost::filesystem::directory_iterator end_itr;

  for ( boost::filesystem::directory_iterator itr ( dir ); itr != end_itr;
        ++itr ) {
    if (boost::filesystem::is_regular (*itr) ) {
      boost::filesystem::path extension = itr->path().extension();
      boost::filesystem::path extension2 = itr->path().stem().extension();
      std::string fileName = itr->path().filename().string();

      if (extension.string() == ".json" && extension2.string() == ".conf") {
        boost::property_tree::ptree moduleConfig;

        boost::property_tree::read_json (itr->path().string(), moduleConfig);

        moduleConfig.add ("configPath", itr->path().parent_path().string() );

        {
          boost::filesystem::path diffpath;

          boost::filesystem::path tmppath = itr->path().parent_path();

          while (tmppath != parentDir) {
            diffpath = tmppath.stem() / diffpath;
            tmppath = tmppath.parent_path();
          }

          tmppath = diffpath;

          boost::property_tree::ptree loadedConfig;
          std::string key = "modules";

          for (auto it = tmppath.begin(); it != tmppath.end(); it ++) {
            key += "." + it->string();
          }

          fileName = fileName.substr (0, fileName.size() - extension.string().size() );
          fileName = fileName.substr (0, fileName.size() - extension2.string().size() );

          key += "." + fileName;

          loadedConfig.put_child (key, moduleConfig);

          mergePropertyTrees (config, loadedConfig);
        }

        GST_INFO ("Loaded module config from: %s", itr->path().string().c_str() );
      }
    } else if (boost::filesystem::is_directory (*itr) ) {
      loadModulesConfigFromDir (config, itr->path(), parentDir);
    }
  }
}

static void
loadModulesConfig (boost::property_tree::ptree &config,
                   const boost::filesystem::path &configFilePath, std::string modulesConfigPath)
{
  std::list <std::string> locations;

  if (modulesConfigPath.empty() ) {
    boost::filesystem::path modulesConfigDir = configFilePath.parent_path() /
        "modules";

    modulesConfigPath = modulesConfigDir.string();
  }

  locations = split (modulesConfigPath, ':');

  for (std::string location : locations) {
    boost::filesystem::path dir (location);

    loadModulesConfigFromDir (config, dir, dir);
  }
}

static std::shared_ptr<Transport>
load_config (boost::property_tree::ptree &config, const std::string &file_name,
             const std::string &modulesConfigPath)
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

  loadModulesConfig (config, configFilePath, modulesConfigPath);

  config.add ("configPath", configFilePath.parent_path().string() );

  GST_INFO ("Configuration loaded successfully");

  std::ostringstream oss;
  boost::property_tree::write_json (oss, config, true);
  std::string jsonConfig = oss.str();

  GST_DEBUG ("Effective loaded config:\n%s", jsonConfig.c_str() );

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

static void
kms_init_dependencies (int argc, char **argv)
{
  Glib::init();

  gst_init (&argc, &argv);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  /* Initialization routine to add commonly used */
  /* attributes to the logging system */
  logging::add_common_attributes();
}

int
main (int argc, char **argv)
{
  sigset_t mask;
  std::shared_ptr <SignalHandler> signalHandler;
  std::shared_ptr<Transport> transport;
  boost::property_tree::ptree config;
  std::string confFile;
  std::string path, logs_path, modulesConfigPath;

  kms_init_dependencies (argc, argv);

  try {
    boost::program_options::options_description desc ("kurento-media-server usage");

    desc.add_options()
    ("help,h", "Display this help message")
    ("version,v", "Display the version number")
    ("list,l", "Lists all available factories")
    ("modules-path,p", boost::program_options::value<std::string>
     (&path), "Path where kurento modules can be found")
    ("conf-file,f", boost::program_options::value<std::string>
     (&confFile)->default_value (DEFAULT_CONFIG_FILE),
     "Configuration file location")
    ("logs-path,d", boost::program_options::value <std::string> (&logs_path),
     "Path where debug files will be stored")
    ("modules-config-path,c",
     boost::program_options::value <std::string> (&modulesConfigPath),
     "Path where modules config files can be found");

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

    if (vm.count ("logs-path") ) {
      if (kms_init_logging (logs_path) ) {
        GST_DEBUG ("Dumping logs to %s", logs_path.c_str() );
      } else {
        GST_WARNING ("Can no set debugging path %s", logs_path.c_str() );
      }
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

  transport = load_config (config, confFile, modulesConfigPath);

  /* Start transport */
  transport->start ();

  GST_INFO ("Mediaserver started");

  loop->run ();

  transport->stop();

  GST_INFO ("Mediaserver stopped");

  return 0;
}
