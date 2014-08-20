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

#include <gst/gst.h>
#include "Processor.hpp"
#include "WebSocketTransport.hpp"
#include <regex>

#define GST_CAT_DEFAULT kurento_websocket_transport
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebSocketTransport"

namespace kurento
{

/* Default config values */
const std::string WEBSOCKET_ADDRESS_DEFAULT = "127.0.0.1";
const uint WEBSOCKET_PORT_DEFAULT = 9090;
const std::string WEBSOCKET_PATH_DEFAULT = "kurento";
const int WEBSOCKET_THREADS_DEFAULT = 10;

static void
check_port (int port)
{
  if (port <= 0 || port > G_MAXUSHORT) {
    throw boost::property_tree::ptree_bad_data ("Invalid port value", port);
  }
}

WebSocketTransport::WebSocketTransport (const boost::property_tree::ptree
                                        &config,
                                        std::shared_ptr<Processor> processor) : processor (processor)
{
  uint port;

  try {
    port = config.get<uint> ("mediaServer.net.websocket.port");
    check_port (port);
  } catch (const boost::property_tree::ptree_error &err) {
    GST_WARNING ("Setting default port %d to websocket",
                 WEBSOCKET_PORT_DEFAULT);
    port = WEBSOCKET_PORT_DEFAULT;
  }

  try {
    path = config.get<std::string> ("mediaServer.net.websocket.path");
  } catch (const boost::property_tree::ptree_error &err) {
    GST_WARNING ("Setting default path %d to websocket",
                 WEBSOCKET_PORT_DEFAULT);
    path = WEBSOCKET_PATH_DEFAULT;
  }

  try {
    n_threads = config.get<uint> ("mediaServer.net.websocket.threads");

    if (n_threads < 1) {
      throw boost::property_tree::ptree_bad_data ("Invalid threads number",
          n_threads);
    }
  } catch (const boost::property_tree::ptree_error &err) {
    GST_WARNING ("Setting default listener threads %d to websocket",
                 WEBSOCKET_THREADS_DEFAULT);
    n_threads = WEBSOCKET_THREADS_DEFAULT;
  }

  server.clear_access_channels (websocketpp::log::alevel::all);
  server.clear_error_channels (websocketpp::log::alevel::all);

  server.init_asio();
  server.set_reuse_addr (true);
  server.set_open_handler (std::bind (&WebSocketTransport::openHandler, this,
                                      std::placeholders::_1) );
  server.set_close_handler (std::bind (&WebSocketTransport::closeHandler, this,
                                       std::placeholders::_1) );
  server.set_message_handler (std::bind (&WebSocketTransport::processMessage,
                                         this, std::placeholders::_1, std::placeholders::_2) );

  server.listen (port);
}

WebSocketTransport::~WebSocketTransport()
{
}

void WebSocketTransport::run()
{
  bool running = true;

  while (running) {
    try {
      server.run();
      running = false;
    } catch (std::exception &e) {
      GST_ERROR ("Unexpected error while running the server: %s", e.what() );
    } catch (...) {
      GST_ERROR ("Unexpected error while running the server");
    }
  }
}

void WebSocketTransport::start ()
{
  server.start_accept();

  for (int i = 0; i < n_threads; i++) {
    threads.push_back (std::thread (std::bind (&WebSocketTransport::run, this) ) );
  }
}

void WebSocketTransport::stop ()
{
  GST_DEBUG ("stop transport");
  server.stop();

  for (int i = 0; i < n_threads; i++) {
    threads[i].join();
  }
}

void WebSocketTransport::processMessage (websocketpp::connection_hdl hdl,
    WebSocketServer::message_ptr msg)
{
  std::string data = msg->get_payload();
  std::string response;

  GST_DEBUG ("Message: >%s<", data.c_str() );
  processor->process (data, response);
  GST_DEBUG ("Response: >%s<", response.c_str() );

  server.send (hdl, response, msg->get_opcode() );
}

void WebSocketTransport::openHandler (websocketpp::connection_hdl hdl)
{
  auto connection = server.get_con_from_hdl (hdl);
  std::string resource = connection->get_resource();

  GST_DEBUG ("Client connected from %s", connection->get_origin().c_str() );

  try {
    if (!std::regex_match (connection->get_resource(),
                           std::regex ("/" + path + "(\?.*)|($)") ) ) {
      try {
        GST_ERROR ("Invalid uri, closing connection");
        server.close (hdl, websocketpp::close::status::protocol_error, "Invalid uri");
      } catch (std::error_code &e) {
        GST_ERROR ("Error: %s", e.message().c_str() );
      }
    }
  } catch (std::regex_error &e) {
    GST_ERROR ("Regex error: %s", e.what() );
  }
}

void WebSocketTransport::closeHandler (websocketpp::connection_hdl hdl)
{
  GST_DEBUG ("Connection closed");
}

WebSocketTransport::StaticConstructor WebSocketTransport::staticConstructor;

WebSocketTransport::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
