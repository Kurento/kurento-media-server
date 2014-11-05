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

#ifndef __BASE_TEST__
#define __BASE_TEST__

#include <websocketpp/connection.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

typedef websocketpp::client<websocketpp::config::asio_client> WebSocketClient;

namespace kurento
{

struct F {
protected:
  F();
  virtual ~F();

  std::string sendMessage (const std::string &message);

  int getId() {
    return id++;
  }

  std::atomic<int> id;

private:
  int pid;

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

  bool sendingMessage;
  std::string response;
  std::condition_variable cond;

  bool receivedMessage = false;

  bool received () {
    return receivedMessage;
  }

  boost::filesystem::path write_config (const boost::filesystem::path &orig,
                                        uint port);
  boost::filesystem::path configDir;
};

} /* kurento */

#endif /* __BASE_TEST__ */
