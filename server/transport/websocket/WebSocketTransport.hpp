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

#ifndef __WEBSOCKET_TRANSPORT_HPP__
#define __WEBSOCKET_TRANSPORT_HPP__

#include "Transport.hpp"
#include "Processor.hpp"

#ifndef _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_STL_
#endif

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <thread>

typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;

namespace kurento
{

class WebSocketTransport: public Transport
{
public:
  WebSocketTransport (const boost::property_tree::ptree &config,
                      std::shared_ptr<Processor> processor);
  virtual ~WebSocketTransport() throw ();
  virtual void start ();
  virtual void stop ();

private:

  void processMessage (websocketpp::connection_hdl hdl,
                       WebSocketServer::message_ptr msg);
  void openHandler (websocketpp::connection_hdl hdl);
  void closeHandler (websocketpp::connection_hdl hdl);
  void run ();

  std::shared_ptr<Processor> processor;

  std::string path;
  WebSocketServer server;
  std::thread thread;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __WEBSOCKET_TRANSPORT_HPP__ */
