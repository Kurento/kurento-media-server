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

  std::cout << "Kurento Media Server version: " << get_version() << std::endl;

  if (moduleManager.getModules().size () > 0) {
    std::cout << "Found modules:" << std::endl;

    for (auto module : moduleManager.getModules() ) {
      std::cout << "\t'" << module.second->getName() << "' version " <<
                module.second->getVersion() << std::endl;
    }
  }
}
