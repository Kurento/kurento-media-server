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

#ifndef WEBSOCKETREGISTRAR_HPP
#define WEBSOCKETREGISTRAR_HPP

#ifndef _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_STL_
#endif

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio.hpp>
#include <thread>
#include <atomic>

namespace kurento
{

typedef websocketpp::client<websocketpp::config::asio> WebSocketClient;
typedef websocketpp::client<websocketpp::config::asio_tls>
SecureWebSocketClient;

class WebSocketRegistrar
{
public:
  WebSocketRegistrar (const std::string &registrarAddress,
                      const std::string &localAddress, ushort localPort,
                      ushort localSecurePort, const std::string &path);
  ~WebSocketRegistrar();

  void start ();
  void stop ();

private:

  std::string localAddress;
  ushort localPort;
  ushort localSecurePort;
  std::string path;

  std::string registrarAddress;
  std::thread thread;
  std::atomic<bool> finished;
  std::atomic<bool> secure;

  std::chrono::milliseconds waitTime;
  std::mutex mutex;
  std::condition_variable cond;

  websocketpp::connection_hdl connection;

  std::shared_ptr<WebSocketClient> client;
  std::shared_ptr<SecureWebSocketClient> secureClient;

  void connectRegistrar ();
  template <typename ClientType>
  void connectionOpen (std::shared_ptr<ClientType> client,
                       websocketpp::connection_hdl hdl);
  template <typename ClientType>
  void receivedMessage (std::shared_ptr<ClientType> client,
                        websocketpp::connection_hdl hdl,
                        typename ClientType::message_ptr msg);
  void closedHandler (websocketpp::connection_hdl hdl);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif // WEBSOCKETREGISTRAR_HPP
