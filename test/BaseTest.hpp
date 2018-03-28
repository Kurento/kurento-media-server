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

#ifndef __BASE_TEST__
#define __BASE_TEST__

#include <websocketpp/connection.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

#include <list>

typedef websocketpp::client<websocketpp::config::asio_client> WebSocketClient;

namespace kurento
{

struct F {
protected:
  F();
  virtual ~F();

  Json::Value sendRequest (const Json::Value &request);
  Json::Value waifForEvent (const std::chrono::seconds timeout);

  int getId()
  {
    return id++;
  }

  std::atomic<int> id{};

  void setWsHost (const std::string &host)
  {
    wsHost = host;
  }

  void setResourceLimit (float limit)
  {
    resourceLimit = limit;
  }

  void stop();
  void start();

private:
  int pid{};

  std::string wsHost = "localhost";

  void start_server ();
  void stop_server ();

  bool initialized = false;
  bool terminate = false;
  std::mutex mutex;

  void on_open (websocketpp::connection_hdl hdl);
  void on_message (websocketpp::connection_hdl hdl,
                   WebSocketClient::message_ptr msg);

  void start_client ();
  void stop_client (std::unique_lock <std::mutex> &lock);

  void create_ws_uri (uint port);
  std::string uri;

  std::shared_ptr <WebSocketClient> client;
  websocketpp::connection_hdl connectionHdl;
  std::thread clientThread;

  Json::Reader reader;

  bool sendingMessage{};
  std::list <Json::Value> recvMessages;
  std::condition_variable cond;

  bool receivedResponse (const std::string &requestId);

  bool receivedEvent ();

  bool isEvent (const Json::Value &message);
  bool isResponse (const Json::Value &message, const std::string &requestId);

  Json::Value getFirstEvent ();
  Json::Value getResponse (const std::string &requestId);

  boost::filesystem::path write_config (const boost::filesystem::path &orig,
                                        uint port);
  boost::filesystem::path configDir;

  float resourceLimit = 1.0;
};

} /* kurento */

#endif /* __BASE_TEST__ */
