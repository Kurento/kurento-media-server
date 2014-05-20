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

#ifndef __HTTP_SERVICE_HPP__
#define __HTTP_SERVICE_HPP__

#include <KmsHttpEPServer.h>
#include "Service.hpp"

namespace kurento
{

class HttpService: public Service
{
public:
  HttpService (Glib::KeyFile &confFile, bool fixedPort = true);
  virtual ~HttpService() throw ();
  virtual void start ();
  virtual void stop ();

private:
  int port;
  std::string address;
  std::string announcedAddr;
  KmsHttpEPServer *server;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __HTTP_SERVICE_HPP__ */
