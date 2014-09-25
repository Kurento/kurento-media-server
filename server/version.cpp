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
#include "version.hpp"

#include <iostream>
#include "modules.hpp"

const char *
get_version ()
{
  return PROJECT_VERSION;
}

void
print_version ()
{
  kurento::ModuleManager &moduleManager = kurento::getModuleManager();

  std::cout << "Version: " << PROJECT_VERSION << std::endl;

  if (moduleManager.getModules().size () > 0) {
    std::cout << "Found modules:" << std::endl;

    for (auto module : moduleManager.getModules() ) {
      std::cout << "\tModule: '" << module.second->getName() << "' version '" <<
                module.second->getVersion() << "'" << std::endl;
    }
  }
}
