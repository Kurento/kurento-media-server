#include "modules.hpp"

namespace kurento
{

static ModuleManager moduleManager;

ModuleManager &getModuleManager ()
{
  return moduleManager;
}

void loadModules (const std::string &path)
{
  moduleManager.loadModulesFromDirectories (path);
}

} /* kurento */
