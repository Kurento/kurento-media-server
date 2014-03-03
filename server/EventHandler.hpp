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

#ifndef __EVENT_HANDLER_HPP__
#define __EVENT_HANDLER_HPP__

#include <sigc++/sigc++.h>
#include <string>
#include <json/json.h>
#include <glibmm.h>

namespace kurento
{

class EventHandler
{
public:
  EventHandler (const std::string &ip, int port);

  virtual ~EventHandler();

  void sendEvent (Json::Value &value) const;

  std::string getId () const {
    return id;
  }

  void setConnection (sigc::connection conn) {
    this->conn = conn;
  }

private:
  static Glib::ThreadPool pool;
  sigc::connection conn;
  std::string ip;
  int port;
  std::string id;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __EVENT_HANDLER_HPP__ */
