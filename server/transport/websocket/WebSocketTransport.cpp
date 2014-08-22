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
#include "WebSocketEventHandler.hpp"
#include <jsonrpc/JsonRpcUtils.hpp>
#include <jsonrpc/JsonRpcConstants.hpp>
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_websocket_transport
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebSocketTransport"

namespace kurento
{

const std::string SESSION_ID = "sessionId";

/* Default config values */
const uint WEBSOCKET_PORT_DEFAULT = 9090;
const std::string WEBSOCKET_PATH_DEFAULT = "kurento";
const int WEBSOCKET_THREADS_DEFAULT = 10;

const std::chrono::seconds KEEP_ALIVE_PERIOD (60);

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

  processor->setEventSubscriptionHandler (std::bind (
      &WebSocketTransport::processSubscription, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) );

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

void WebSocketTransport::keepAliveSessions()
{
  std::unique_lock<std::mutex> lock (mutex);

  while (isRunning() ) {
    for (auto c : connections) {
      GST_INFO ("Keep alive %s", c.first.c_str() );

      try {
        processor->keepAliveSession (c.first);
      } catch (KurentoException &e) {
        if (e.getCode() == INVALID_SESSION) {
          GST_INFO ("Session should be removed %s", c.first.c_str() );
          // TODO: Remove session
        } else {
          throw e;
        }
      }
    }

    cond.wait_for (lock, KEEP_ALIVE_PERIOD);
  }
}

void WebSocketTransport::start ()
{

  server.start_accept();

  for (int i = 0; i < n_threads; i++) {
    threads.push_back (std::thread (std::bind (&WebSocketTransport::run, this) ) );
  }

  std::unique_lock<std::mutex> lock (mutex);
  running = true;
  keepAliveThread = std::thread (std::bind (
                                   &WebSocketTransport::keepAliveSessions, this) );
}

void WebSocketTransport::stop ()
{
  std::unique_lock<std::mutex> lock (mutex);
  running = false;
  cond.notify_all();
  lock.unlock();

  GST_DEBUG ("stop transport");
  server.stop();

  for (int i = 0; i < n_threads; i++) {
    threads[i].join();
  }

  keepAliveThread.join();
}

static void
injectSessionId (std::string &request, const std::string &sessionId)
{
  try {
    Json::Reader reader;
    Json::Value req;
    Json::Value params;

    reader.parse (request, req);
    JsonRpc::getValue (req, JSON_RPC_PARAMS, params);

    try {
      std::string oldSessionId;
      JsonRpc::getValue (req, SESSION_ID, oldSessionId);
    } catch (JsonRpc::CallException &e) {
      Json::FastWriter writer;
      // There is no sessionId, inject it
      GST_WARNING ("Injecting sessionId %s", sessionId.c_str() );
      params[SESSION_ID] = sessionId;
      req[JSON_RPC_PARAMS] = params;

      request = writer.write (req);
    }
  } catch (JsonRpc::CallException &ex) {

  }
}

static std::string
getSessionId (const std::string &request, const std::string &response)
{
  std::string sessionId;

  try {
    try {
      Json::Reader reader;
      Json::Value resp;
      Json::Value result;

      reader.parse (response, resp);

      JsonRpc::getValue (resp, JSON_RPC_RESULT, result);
      JsonRpc::getValue (result, SESSION_ID, sessionId);
    } catch (JsonRpc::CallException &ex) {
      Json::Reader reader;
      Json::Value req;
      Json::Value params;

      reader.parse (request, req);

      JsonRpc::getValue (req, JSON_RPC_PARAMS, params);
      JsonRpc::getValue (params, SESSION_ID, sessionId);
    }
  } catch (JsonRpc::CallException &e) {
    /* We could not get some of the required parameters. Ignore */
  }

  return sessionId;
}

websocketpp::connection_hdl
WebSocketTransport::getConnection (const std::string &sessionId)
{
  try {
    std::unique_lock<std::mutex> lock (mutex);
    return connections.at (sessionId);
  } catch (std::out_of_range &e) {
    throw std::out_of_range ("Connection not found for sessionId: " + sessionId);
  }
}

void WebSocketTransport::storeConnection (const std::string &request,
    const std::string &response, websocketpp::connection_hdl connection)
{
  std::string sessionId = getSessionId (request, response);

  if (!sessionId.empty() ) {
    std::unique_lock<std::mutex> lock (mutex);
    bool needsWrite = false;

    try {
      websocketpp::connection_hdl conn = connections.at (sessionId);

      if (conn.lock() != connection.lock() ) {
        GST_WARNING ("Erasing old connection associated with: %s", sessionId.c_str() );
        connectionsReverse.erase (conn);
        connections.erase (sessionId);
        needsWrite = true;
      }
    } catch (std::out_of_range &e) {
      /* Ignore */
      needsWrite = true;
    }

    try {
      std::string oldSession = connectionsReverse.at (connection);

      if (oldSession != sessionId) {
        GST_WARNING ("Erasing old sessionId %s associated with current connection",
                     oldSession.c_str() );
        connectionsReverse.erase (connection);
        connections.erase (oldSession);
        needsWrite = true;
      }

    } catch (std::out_of_range &e) {
      needsWrite = true;
    }

    if (needsWrite) {
      GST_DEBUG ("Asociating session %s", sessionId.c_str() );
      connections[sessionId] = connection;
      connectionsReverse[connection] = sessionId;
      processor->keepAliveSession (sessionId);
    }
  }
}

void WebSocketTransport::processMessage (websocketpp::connection_hdl hdl,
    WebSocketServer::message_ptr msg)
{
  std::string request = msg->get_payload();
  std::string response;

  try {
    std::string sessionId;

    sessionId = connectionsReverse.at (hdl);
    injectSessionId (request, sessionId);
  } catch (std::out_of_range &e) {
    /* Ignore, there is no previous sessionId */
  }

  GST_DEBUG ("Message: >%s<", request.c_str() );
  processor->process (request, response);
  GST_DEBUG ("Response: >%s<", response.c_str() );

  storeConnection (request, response, hdl);

  try {
    server.send (hdl, response, websocketpp::frame::opcode::TEXT);
  } catch (websocketpp::lib::error_code &e) {
    GST_ERROR ("Could not send response to client: %s", e.message().c_str() );
  }
}

void WebSocketTransport::openHandler (websocketpp::connection_hdl hdl)
{
  auto connection = server.get_con_from_hdl (hdl);
  std::string resource = connection->get_resource();

  GST_DEBUG ("Client connected from %s", connection->get_origin().c_str() );

  if (resource.size() >= 1 && resource[0] == '/') {
    resource = resource.substr (1);
  }

  resource = resource.substr (0, resource.find_first_of ('?') );

  if (resource != path) {
    try {
      GST_ERROR ("Invalid path \"%s\", closing connection",
                 connection->get_resource().c_str() );
      server.close (hdl, websocketpp::close::status::protocol_error, "Invalid path");
    } catch (std::error_code &e) {
      GST_ERROR ("Error: %s", e.message().c_str() );
    }
  }
}

std::string
WebSocketTransport::processSubscription (std::shared_ptr< MediaObjectImpl > obj,
    const std::string &sessionId,
    const std::string &eventType,
    const Json::Value &params)
{
  std::string subscriptionId;
  std::shared_ptr <EventHandler> handler;

  handler = std::shared_ptr <EventHandler> (new WebSocketEventHandler (obj,
            shared_from_this(), sessionId) );

  subscriptionId = processor->connectEventHandler (obj, sessionId, eventType,
                   handler);

  return subscriptionId;
}

void WebSocketTransport::closeHandler (websocketpp::connection_hdl hdl)
{
  GST_DEBUG ("Connection closed");

  try {
    std::unique_lock<std::mutex> lock (mutex);
    std::string sessionId = connectionsReverse.at (hdl);

    GST_DEBUG ("Erasing connection associated with: %s", sessionId.c_str() );
    connections.erase (sessionId);
    connectionsReverse.erase (hdl);
  } catch (std::out_of_range &e) {
    /* Ignore */
  }
}

WebSocketTransport::StaticConstructor WebSocketTransport::staticConstructor;

WebSocketTransport::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
