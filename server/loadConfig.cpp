/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
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

#include <gst/gst.h>

#include "loadConfig.hpp"

#include <iostream>
#include <queue>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <utility>

#define GST_CAT_DEFAULT kurento_load_config
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoLoadConfig"

namespace kurento
{

class ParseException : public virtual std::exception
{
public:
  ParseException(std::string message) : message(std::move(message)){};

  ~ParseException() override = default;

  const char *what() const noexcept override { return message.c_str(); };

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

void
mergePropertyTrees (boost::property_tree::ptree &ptMerged,
                    const boost::property_tree::ptree &ptSecond, int level )
{
  // Value or object or array
  if (level > 0 && ptSecond.empty() ) {
    // Copy value
    ptMerged = ptSecond;
  } else if (level > 0 && ptSecond.count (std::string() ) == ptSecond.size() ) {
    // Copy array
    ptMerged = ptSecond;
  } else {
    auto it = ptSecond.begin();

    for (; it != ptSecond.end(); ++it) {
      boost::property_tree::ptree child = ptMerged.get_child (it->first.data(),
                                          boost::property_tree::ptree() );
      mergePropertyTrees (child, it->second, level + 1);

      ptMerged.erase (it->first.data() );
      ptMerged.add_child (it->first.data(), child);
    }
  }
}

static std::string
loadFile (boost::property_tree::ptree &config,
          const boost::filesystem::path &fspath)
{
  const std::string path = fspath.string();
  const std::string extension1 = fspath.stem().extension().string();
  const std::string extension2 = fspath.extension().string();

  boost::property_tree::ptree readConfig;

  if (extension1 == ".conf") {
    if (extension2 == ".json") {
      try {
        boost::property_tree::read_json (path, readConfig);
      } catch (boost::property_tree::json_parser_error &e) {
        throw ParseException (std::string("JSON parse error: ") + e.what());
      }
    } else if (extension2 == ".info") {
      try {
        boost::property_tree::read_info (path, readConfig);
      } catch (boost::property_tree::info_parser_error &e) {
        throw ParseException (std::string("INFO parse error: ") + e.what());
      }
    } else if (extension2 == ".ini") {
      try {
        boost::property_tree::read_ini (path, readConfig);
      } catch (boost::property_tree::ini_parser_error &e) {
        throw ParseException (std::string("INI parse error: ") + e.what());
      }
    } else if (extension2 == ".xml") {
      try {
        boost::property_tree::read_xml (path, readConfig);
      } catch (boost::property_tree::xml_parser_error &e) {
        throw ParseException (std::string("XML parse error: ") + e.what());
      }
    } else {
      throw ParseException ("Unknown conf format: " + extension2);
    }
  } else {
    throw ParseException ("Unknown file type: " + fspath.filename().string());
  }

  mergePropertyTrees (config, readConfig);

  config.put ("configPath", fspath.parent_path().string() );

  std::string fileName = fspath.filename().string();
  fileName = fileName.substr (0, fileName.size() - extension1.size()
      - extension2.size());

  return fileName;
}

static std::string
diffPathToKey (const boost::filesystem::path &path,
               const boost::filesystem::path &ancestorPath)
{
  boost::filesystem::path diffpath;

  boost::filesystem::path tmppath = path;

  while (tmppath != ancestorPath) {
    diffpath = tmppath.stem() / diffpath;
    tmppath = tmppath.parent_path();
  }

  tmppath = diffpath;

  boost::property_tree::ptree loadedConfig;
  std::string key;

  for (auto it = tmppath.begin(); it != tmppath.end(); it ++) {
    if (key.empty() ) {
      key = it->string();
    } else {
      key += "." + it->string();
    }
  }

  return key;
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
      const std::string pathStr = itr->path().string();
      try {
        boost::property_tree::ptree moduleConfig;
        std::string fileName = loadFile (moduleConfig, itr->path() );
        std::string key = diffPathToKey (itr->path().parent_path(), parentDir);
        boost::property_tree::ptree loadedConfig;

        key = key.empty() ? "modules" :  "modules." + key;
        key += "." + fileName;

        loadedConfig.put_child (key, moduleConfig);
        mergePropertyTrees (config, loadedConfig);

        GST_INFO ("Loaded module config: %s", pathStr.c_str());
      } catch (ParseException &e) {
        GST_WARNING ("Error loading config: %s, %s", pathStr.c_str(), e.what());
        std::cerr << "Error loading config: " << pathStr << ", " << e.what()
            << std::endl;
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
    dir.normalize();

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
  std::string infoConfig = oss.str();

  GST_INFO ("Loaded config in effect:\n%s", infoConfig.c_str() );
}

} /* kurento */

static void init_debug() __attribute__((constructor));

static void init_debug() {
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
