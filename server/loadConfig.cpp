/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
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

#include <gst/gst.h>

#include "loadConfig.hpp"

#include <queue>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#define GST_CAT_DEFAULT kurento_load_config
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoLoadConfig"

namespace kurento
{

class ParseException : public virtual std::exception
{
public:
  ParseException (const std::string &message) : message (message) {};

  virtual ~ParseException() {};

  virtual const char *what() const noexcept
  {
    return message.c_str();
  };

  const std::string &getMessage() const
  {
    return message;
  };

private:
  std::string message;
};

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

static std::string
loadFile (boost::property_tree::ptree &config,
          const boost::filesystem::path &file)
{
  boost::filesystem::path extension = file.extension();
  boost::filesystem::path extension2 = file.stem().extension();
  std::string fileName = file.filename().string();
  boost::property_tree::ptree readConfig;

  if (extension2.string() == ".conf") {
    if (extension.string() == ".json") {
      boost::property_tree::read_json (file.string(), readConfig);
    } else if (extension.string() == ".info") {
      boost::property_tree::read_info (file.string(), readConfig);
    } else if (extension.string() == ".ini") {
      boost::property_tree::read_ini (file.string(), readConfig);
    } else if (extension.string() == ".xml") {
      boost::property_tree::read_xml (file.string(), readConfig);
    } else {
      throw ParseException ("Unknonw file format");
    }
  } else {
    throw ParseException ("Unknonw file format");
  }

  mergePropertyTrees (config, readConfig);

  config.put ("configPath", file.parent_path().string() );

  fileName = fileName.substr (0, fileName.size() - extension.string().size() );
  fileName = fileName.substr (0, fileName.size() - extension2.string().size() );

  return fileName;
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
      try {
        boost::property_tree::ptree moduleConfig;
        std::string fileName = loadFile (moduleConfig, itr->path() );

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

        key += "." + fileName;

        loadedConfig.put_child (key, moduleConfig);

        mergePropertyTrees (config, loadedConfig);

        GST_INFO ("Loaded module config from: %s", itr->path().string().c_str() );
      } catch (ParseException &e) {
        // Ignore this exceptions here
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

void
loadConfig (boost::property_tree::ptree &config, const std::string &file_name,
            const std::string &modulesConfigPath)
{
  boost::filesystem::path configFilePath (file_name);
  GST_INFO ("Reading configuration from: %s", file_name.c_str () );

  try {
    loadFile (config, configFilePath);
  } catch (ParseException &e) {
    GST_ERROR ("Error reading configuration: %s", e.what() );
    std::cerr << "Error reading configuration: " << e.what() << std::endl;
    exit (1);
  } catch (boost::property_tree::ptree_error &e) {
    GST_ERROR ("Error reading configuration: %s", e.what() );
    std::cerr << "Error reading configuration: " << e.what() << std::endl;
    exit (1);
  }

  loadModulesConfig (config, configFilePath, modulesConfigPath);

  GST_INFO ("Configuration loaded successfully");

  std::ostringstream oss;
  boost::property_tree::write_json (oss, config, true);
  std::string jsonConfig = oss.str();

  GST_DEBUG ("Effective loaded config:\n%s", jsonConfig.c_str() );
}

} /* kurento */

static void init_debug (void) __attribute__ ( (constructor) );

static void
init_debug (void)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
