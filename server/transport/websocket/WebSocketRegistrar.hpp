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

#ifndef WEBSOCKETREGISTRAR_HPP
#define WEBSOCKETREGISTRAR_HPP

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
  WebSocketRegistrar(const std::string &registrarAddress,
                     std::string localAddress, ushort localPort,
                     ushort localSecurePort, std::string path);
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
  std::atomic<bool> finished{};
  std::atomic<bool> secure{};

  std::chrono::milliseconds waitTime{};
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
