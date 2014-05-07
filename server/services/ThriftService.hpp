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

#ifndef __THRIFT_SERVICE_HPP__
#define __THRIFT_SERVICE_HPP__

#include "Service.hpp"
#include <server/TNonblockingServer.h>

namespace kurento
{

class ThriftService: public Service
{
public:
  ThriftService (Glib::KeyFile &confFile);
  virtual ~ThriftService() throw () {};
  virtual void start (std::function<void (GError *err) > func);
  virtual void stop (std::function<void (GError *err) > func);

private:
  int port;
  std::string address;
  apache::thrift::server::TNonblockingServer *server;
  Glib::Thread *thread;

  void start_thrift ();

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __THRIFT_SERVICE_HPP__ */