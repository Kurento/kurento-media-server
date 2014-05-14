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

#ifndef __THRIFT_EVENT_HANDLER_HPP__
#define __THRIFT_EVENT_HANDLER_HPP__

#include <memory>
#include <string>
#include <json/json.h>
#include <glibmm.h>
#include <EventHandler.hpp>

namespace kurento
{

class ThriftEventHandler : public EventHandler
{
public:
  ThriftEventHandler (std::shared_ptr<MediaObject> obj,
                      const std::string &sessionId, const std::string &ip,
                      int port);

  virtual ~ThriftEventHandler();

  virtual void sendEvent (Json::Value &value) const;

private:
  static Glib::ThreadPool pool;

  std::string ip;
  int port;
  std::string sessionId;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __THRIFT_EVENT_HANDLER_HPP__ */
