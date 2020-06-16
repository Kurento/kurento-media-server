/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#ifndef __WEBSOCKET_TRANSPORT_HPP__
#define __WEBSOCKET_TRANSPORT_HPP__

#include "Transport.hpp"
#include "Processor.hpp"

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
  // Constructor methods
  void initWebSocket(const boost::property_tree::ptree &config);
  void initSecureWebSocket(const boost::property_tree::ptree &config);
  void initRegistrar (const boost::property_tree::ptree &config);

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
                        websocketpp::connection_hdl connection, bool secure, std::string &sessionId);

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
  bool hasInsecureServer = false;
  bool hasSecureServer = false;
  std::vector<std::thread> threads;
  std::thread keepAliveThread;
  bool running = false;
  std::condition_variable_any cond;
  std::shared_ptr <WebSocketRegistrar> registrar;

  std::map <std::string, std::weak_ptr<kurento::EventHandler>> handlers;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __WEBSOCKET_TRANSPORT_HPP__ */
