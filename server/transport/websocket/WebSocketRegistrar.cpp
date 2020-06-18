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

#include "WebSocketRegistrar.hpp"
#include <json/json.h>
#include <gst/gst.h>

#include <memory>
#include <utility>

#define GST_CAT_DEFAULT kurento_websocket_registrar
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebSocketRegistrar"

namespace kurento
{

const std::chrono::milliseconds DEFAULT_WAIT_TIME (100);
const std::chrono::seconds MAX_WAIT_TIME (10);

typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

WebSocketRegistrar::WebSocketRegistrar(const std::string &registrarAddress,
                                       std::string localAddress,
                                       ushort localPort, ushort localSecurePort,
                                       std::string path)
    : localAddress(std::move(localAddress)), localPort(localPort),
      localSecurePort(localSecurePort), path(std::move(path)),
      registrarAddress(registrarAddress) {
  GST_INFO ("Registrar will be performed to: %s", registrarAddress.c_str () );
}

WebSocketRegistrar::~WebSocketRegistrar ()
{
  if (!finished) {
    stop();
  }
}

void
WebSocketRegistrar::start()
{
  if (registrarAddress.empty() || localAddress.empty () ) {
    return;
  }

  waitTime = DEFAULT_WAIT_TIME;
  finished = false;

  thread = std::thread (std::bind (&WebSocketRegistrar::connectRegistrar, this) );
}

void
WebSocketRegistrar::stop()
{
  if (registrarAddress.empty() || localAddress.empty () ) {
    return;
  }

  finished = true;

  try {
    if (secure) {
      secureClient->close (connection, websocketpp::close::status::going_away,
                           "terminating");
      secureClient->get_io_service().stop();
    } else {
      client->close (connection, websocketpp::close::status::going_away,
                     "terminating");
      client->get_io_service().stop();
    }
  } catch (...) {

  }

  std::unique_lock<std::mutex> lock (mutex);
  cond.notify_all ();
  lock.unlock ();

  if (thread.get_id () != std::this_thread::get_id () ) {
    thread.join();
  }
}

void
WebSocketRegistrar::connectRegistrar ()
{
  websocketpp::lib::error_code ec;

  if (registrarAddress.empty () ) {
    return;
  }

  while (!finished) {
    boost::asio::io_service ios;

    GST_INFO ("Connecting registrar");

    if (registrarAddress.size() >= 3 && registrarAddress.substr (0, 3) == "wss") {
      secureClient = std::make_shared<SecureWebSocketClient>();
      secureClient->clear_access_channels (websocketpp::log::alevel::all);
      secureClient->clear_error_channels (websocketpp::log::elevel::all);

      // Register our handlers
      secureClient->set_open_handler (std::bind ( (void (WebSocketRegistrar::*) (
                                        std::shared_ptr<SecureWebSocketClient>,
                                        websocketpp::connection_hdl) ) &WebSocketRegistrar::connectionOpen, this,
                                      secureClient, std::placeholders::_1) );
      secureClient->set_message_handler (std::bind ( (void (WebSocketRegistrar::*) (
                                           std::shared_ptr<SecureWebSocketClient>, websocketpp::connection_hdl,
                                           SecureWebSocketClient::message_ptr) ) &WebSocketRegistrar::receivedMessage,
                                         this, secureClient, std::placeholders::_1, std::placeholders::_2) );
      secureClient->set_close_handler (std::bind (&WebSocketRegistrar::closedHandler,
                                       this, std::placeholders::_1) );
      secureClient->set_tls_init_handler ( [] (websocketpp::connection_hdl hdl) ->
      context_ptr {
        context_ptr ctx (new boost::asio::ssl::context (boost::asio::ssl::context::tlsv12) );

        try {
          ctx->set_options (boost::asio::ssl::context::default_workarounds |
          boost::asio::ssl::context::no_sslv2 |
          boost::asio::ssl::context::single_dh_use);
        } catch (std::exception &e)
        {
          GST_ERROR ("Error setting tls on registrar connection: %s", e.what() );
        }

        return ctx;
      });

      // Initialize ASIO
      secureClient->init_asio (&ios);

      SecureWebSocketClient::connection_ptr con = secureClient->get_connection (
            registrarAddress, ec);

      if (con) {
        secure = true;
        secureClient->connect (con);
        secureClient->run();
      } else {
        GST_ERROR ("Cannot create connection %s", ec.message().c_str() );
      }
    } else {
      client = std::make_shared<WebSocketClient>();
      client->clear_access_channels (websocketpp::log::alevel::all);
      client->clear_error_channels (websocketpp::log::elevel::all);

      // Register our handlers
      client->set_open_handler (std::bind ( (void (WebSocketRegistrar::*) (
          std::shared_ptr<WebSocketClient>,
          websocketpp::connection_hdl) ) &WebSocketRegistrar::connectionOpen, this,
                                            client, std::placeholders::_1) );
      client->set_message_handler (std::bind ( (void (WebSocketRegistrar::*) (
                                     std::shared_ptr<WebSocketClient>, websocketpp::connection_hdl,
                                     WebSocketClient::message_ptr) ) &WebSocketRegistrar::receivedMessage, this,
                                   client, std::placeholders::_1, std::placeholders::_2) );
      client->set_close_handler (std::bind (&WebSocketRegistrar::closedHandler, this,
                                            std::placeholders::_1) );

      // Initialize ASIO
      client->init_asio (&ios);

      WebSocketClient::connection_ptr con = client->get_connection (registrarAddress,
                                            ec);

      if (con) {
        secure = false;
        client->connect (con);
        client->run();
      } else {
        GST_ERROR ("Cannot create connection %s", ec.message().c_str() );
      }
    }

    if (finished) {
      break;
    }

    GST_INFO ("Registrar disconnected, trying to reconnect in %" G_GINT64_FORMAT
              " ms", waitTime.count () );

    std::unique_lock<std::mutex> lock (mutex);
    cond.wait_for (lock, waitTime);
    lock.unlock ();

    if (waitTime < (MAX_WAIT_TIME / 2) ) {
      waitTime = waitTime * 2;
    } else {
      waitTime = MAX_WAIT_TIME;
    }
  }

  GST_INFO ("Terminating");
}

template <typename ClientType>
void
WebSocketRegistrar::connectionOpen (std::shared_ptr<ClientType> client,
                                    websocketpp::connection_hdl hdl)
{
  Json::Value req;
  Json::Value params;
  std::string request;

  waitTime = DEFAULT_WAIT_TIME;
  connection = hdl;

  req["jsonrpc"] = "2.0";
  req["method"] = "register";


  if (localSecurePort > 0) {
    params["ws"] = "wss://" + localAddress + ":" + std::to_string (
                     localSecurePort) + "/" + path;
  } else {
    params["ws"] = "ws://" + localAddress + ":" + std::to_string (
                     localPort) + "/" + path;
  }

  req["params"] = params;

  Json::StreamWriterBuilder writerFactory;
  writerFactory["indentation"] = "";
  request = Json::writeString (writerFactory, req);
  GST_DEBUG ("Registrar open, sending message: %s", request.c_str() );

  try {
    client->send (hdl, request, websocketpp::frame::opcode::TEXT);
  } catch (websocketpp::exception &e) {
    GST_ERROR ("Cannot send message to remote");
  }
}

void
WebSocketRegistrar::closedHandler (websocketpp::connection_hdl hdl)
{
  GST_DEBUG ("Registrar closed");
}

template <typename ClientType>
void
WebSocketRegistrar::receivedMessage (std::shared_ptr<ClientType> client,
                                     websocketpp::connection_hdl hdl,
                                     typename ClientType::message_ptr msg)
{
  GST_DEBUG ("Message: %s", msg->get_payload().c_str() );
}

WebSocketRegistrar::StaticConstructor WebSocketRegistrar::staticConstructor;

WebSocketRegistrar::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
