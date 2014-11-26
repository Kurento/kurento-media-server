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

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <thread>
#include <condition_variable>

typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;
typedef websocketpp::server<websocketpp::config::asio_tls>
SecureWebSocketServer;

namespace kurento
{

class WebSocketRegistrar;

class WebSocketTransport: public Transport,
  public std::enable_shared_from_this<WebSocketTransport>
{
public:
  WebSocketTransport (const boost::property_tree::ptree &config,
                      std::shared_ptr<Processor> processor);
  virtual ~WebSocketTransport() throw ();
  virtual void start ();
  virtual void stop ();

  void send (const std::string &sessionId, const std::string &message);

private:

  websocketpp::connection_hdl getConnection (const std::string &sessionId);

  template <typename ServerType>
  void processMessage (ServerType *s, websocketpp::connection_hdl hdl,
                       typename ServerType::message_ptr msg);
  template <typename ServerType>
  void openHandler (ServerType *s, websocketpp::connection_hdl hdl);
  void closeHandler (websocketpp::connection_hdl hdl);
  void run ();

  virtual std::string processSubscription (std::shared_ptr<MediaObjectImpl> obj,
      const std::string &sessionId, const std::string &eventType,
      const Json::Value &params);

  void storeConnection (const std::string &request, const std::string &response,
                        websocketpp::connection_hdl connection, bool secure);

  void keepAliveSessions ();

  bool isRunning ()
  {
    return running;
  }

  std::shared_ptr<Processor> processor;

  std::map <std::string, websocketpp::connection_hdl> connections;
  std::map <std::string, bool> secureConnections;
  std::map <websocketpp::connection_hdl, std::string,
      std::owner_less<websocketpp::connection_hdl>> connectionsReverse;
  std::recursive_mutex mutex;

  int n_threads;
  std::string path;
  boost::asio::io_service ios;
  WebSocketServer server;
  SecureWebSocketServer secureServer;
  bool hasSecureServer = false;
  std::vector<std::thread> threads;
  std::thread keepAliveThread;
  bool running = false;
  std::condition_variable_any cond;
  std::shared_ptr <WebSocketRegistrar> registrar;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __WEBSOCKET_TRANSPORT_HPP__ */
