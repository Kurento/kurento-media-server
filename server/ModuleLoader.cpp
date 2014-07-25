/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#include <gmodule.h>
#include <gst/gst.h>
#include <vector>
#include <sstream>

#define GST_CAT_DEFAULT kurento_module_loader
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoModuleLoader"

// TODO: This is a temporal solution until plugin path gets defined by
// configuration or global variables or Makefiles
#define DEFAULT_MODULES_PATH "modules:/usr/lib/kurento/modules"

namespace kurento
{

static void
init_debugging (void) __attribute__ ( (constructor) );

static void
init_debugging (void)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

std::vector<std::string> split (const std::string &s, char delim)
{
  std::vector<std::string> elems;
  std::stringstream ss (s);
  std::string item;

  while (std::getline (ss, item, delim) ) {
    elems.push_back (item);
  }

  return elems;
}

static bool
load_module (std::string &name, std::map<std::string, KurentoModule *> &modules)
{
  GModule *module;
  KurentoModule *kurento_module = NULL;
  std::string module_name;

  module = g_module_open (name.c_str(), G_MODULE_BIND_LAZY);

  if (module == NULL) {
    return false;
  }

  if (!g_module_symbol (module, KURENTO_MODULE_DESCRIPTOR,
                        (gpointer *) &kurento_module) ) {
    GST_ERROR ("Error loading symbol %s from library %s",
               KURENTO_MODULE_DESCRIPTOR, name.c_str() );
    goto error;
  }

  if (kurento_module == NULL) {
    goto error;
  }

  module_name = kurento_module->get_name();

  if (modules.find (module_name) != modules.end() ) {
    GST_ERROR ("Module %s already loaded from another library not loading "
               "from %s", module_name.c_str(), name.c_str() );
    goto error;
  }

  modules[module_name] = kurento_module;

  g_module_make_resident (module);

  GST_INFO ("Loading module %s from  %s", module_name.c_str(),
            name.c_str() );

  return true;

error:

  g_module_close (module);

  return false;
}

static void
load_modules_from_dir (std::string &location,
                       std::map<std::string, KurentoModule *> &modules)
{
  DIR *dir;
  struct dirent *ent;
  std::string name;

  GST_TRACE ("Looking for modules in %s", location.c_str() );

  dir = opendir (location.c_str() );

  if (dir == NULL) {
    GST_ERROR ("Unable to load modules from: %s", location.c_str() );
    return;
  }

  /* print all the files and directories within directory */
  while ( (ent = readdir (dir) ) != NULL) {
    name = ent->d_name;

    if (ent->d_type == DT_REG) {
      std::string name = location + "/" + ent->d_name;
      load_module (name, modules);
    } else if (ent->d_type == DT_DIR && "." != name && ".." != name) {
      std::string dirName = location + "/" + ent->d_name;

      load_modules_from_dir (dirName, modules);
    } else if (ent->d_type == DT_LNK) {
      // TODO: Follow sym link and try to load plugins
    }
  }

  closedir (dir);
}

std::map<std::string, KurentoModule *>
load_modules ()
{
  std::string modulesPath = DEFAULT_MODULES_PATH;
  std::vector <std::string> locations;
  std::map <std::string, KurentoModule *> modules;

  locations = split (modulesPath, ':');

  for (std::string location : locations) {
    load_modules_from_dir (location, modules);
  }

  return modules;
}

}
