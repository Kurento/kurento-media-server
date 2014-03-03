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

#ifndef __OBJECT_REGISTRAR_HPP__
#define __OBJECT_REGISTRAR_HPP__

#include <jsoncpp/json/json.h>
#include <map>
#include <memory>

namespace kurento
{

class MediaObject;

class Factory
{
public:
  Factory() {};
  virtual ~Factory() {};

  virtual std::shared_ptr<MediaObject> createObject (const Json::Value
      &params) = 0;

  static std::shared_ptr<MediaObject> getObject (const std::string &id);

  virtual std::string getName() = 0;
};

class ObjectRegistrar
{

public:
  ObjectRegistrar() {};
  ~ObjectRegistrar() {};

  void registerFactory (std::shared_ptr<Factory> factory);
  std::shared_ptr<Factory> getFactory (const std::string &factoryName);

private:
  std::map<std::string, std::shared_ptr<Factory>> factories;
};

extern std::shared_ptr<ObjectRegistrar> objectRegistrar;

} /* kurento */

#endif /* __OBJECT_REGISTRAR_HPP__ */
