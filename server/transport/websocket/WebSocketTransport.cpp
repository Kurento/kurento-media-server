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

#include <gst/gst.h>
#include "Processor.hpp"
#include "WebSocketTransport.hpp"
#include "WebSocketEventHandler.hpp"
#include "WebSocketRegistrar.hpp"
#include <jsonrpc/JsonRpcUtils.hpp>
#include <jsonrpc/JsonRpcConstants.hpp>
#include <KurentoException.hpp>
#include <MediaSet.hpp>

#include <UUIDGenerator.hpp>

#include <boost/filesystem.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>

#include <memory>
#include <type_traits>

#define GST_CAT_DEFAULT kurento_websocket_transport
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebSocketTransport"

typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

namespace kurento
{

const std::string DEFAULT_LOCAL_ADDRESS = "localhost";

/* Default config values */
const bool WEBSOCKET_IPV6_DEFAULT = true;
const ushort WEBSOCKET_PORT_DEFAULT = 8888;
const std::string WEBSOCKET_PATH_DEFAULT = "kurento";
const int WEBSOCKET_THREADS_DEFAULT = 10;
const int WEBSOCKET_CONNQUEUE_DEFAULT =
  boost::asio::socket_base::max_connections;

class configuration_exception : public std::exception
{
public:
  configuration_exception (std::string message)
  {
    this->message = message;
  }

  const char *what() const _GLIBCXX_USE_NOEXCEPT override {
    return message.c_str();
  }

private:
  std::string message;
};

WebSocketTransport::WebSocketTransport (const boost::property_tree::ptree
                                        &config,
                                        std::shared_ptr<Processor> processor) :
  processor (processor)
{
  bool ipv6;
  ushort port;
  ushort securePort;
  int connqueue;
  std::string registrarAddress;
  std::string localAddress;

  ipv6 = config.get<bool> ("mediaServer.net.websocket.ipv6",
                           WEBSOCKET_IPV6_DEFAULT);

  port = config.get<ushort> ("mediaServer.net.websocket.port",
                             WEBSOCKET_PORT_DEFAULT);

  securePort = config.get<ushort> ("mediaServer.net.websocket.secure.port", 0);

  path = config.get<std::string> ("mediaServer.net.websocket.path",
                                  WEBSOCKET_PATH_DEFAULT);

  connqueue = config.get<uint> ("mediaServer.net.websocket.connqueue",
                                WEBSOCKET_CONNQUEUE_DEFAULT);

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

  /* Configure server */
  server.clear_access_channels (websocketpp::log::alevel::all);
  server.clear_error_channels (websocketpp::log::alevel::all);

  server.init_asio (&ios);
  server.set_listen_backlog (connqueue);
  server.set_reuse_addr (true);
  server.set_open_handler (std::bind ( (void (WebSocketTransport::*) (
                                          WebSocketServer *, websocketpp::connection_hdl) )
                                       &WebSocketTransport::openHandler, this,
                                       &server, std::placeholders::_1) );
  server.set_close_handler (std::bind (&WebSocketTransport::closeHandler, this,
                                       std::placeholders::_1) );
  server.set_message_handler (std::bind ( (void (WebSocketTransport::*) (
      WebSocketServer *, websocketpp::connection_hdl,
      WebSocketServer::message_ptr) ) &WebSocketTransport::processMessage,
                                          this, &server, std::placeholders::_1,
                                          std::placeholders::_2) );

  if (ipv6) {
    try {
      server.listen (boost::asio::ip::tcp::v6(), port);
    } catch (websocketpp::exception &e) {
      GST_ERROR ("Error starting listen for websocket transport on port %d: %s using IPv6, trying IPv4", port,
                e.what() );
      ipv6 = false;
    }
  }

  if (!ipv6) {
    try {
        server.listen (boost::asio::ip::tcp::v4(), port);
    } catch (websocketpp::exception &e) {
      GST_ERROR ("Error starting listen for websocket transport on port %d: %s using IPv4", port,
                e.what() );
      exit (1);
    }
  }

  websocketpp::lib::asio::error_code ep_err;
  websocketpp::lib::asio::ip::tcp::endpoint ep;

  ep = server.get_local_endpoint (ep_err);
  if (!ep_err) {
    GST_INFO ("WebSocket server listening on address '%s', port %u",
        ep.address().to_string().c_str(), ep.port());
  }

  /* Configure secure server if enabled */
  if (securePort != 0) {
    try {
      std::string password;
      boost::filesystem::path certificateFile;

      try {
        password =
          config.get<std::string> ("mediaServer.net.websocket.secure.password");
      } catch (const boost::property_tree::ptree_error &err) {
        throw configuration_exception ("Cannot get password for secure websocket");
      }

      try {
        certificateFile = boost::filesystem::path (
                            config.get<std::string> ("mediaServer.net.websocket.secure.certificate") );

        if (certificateFile.is_relative() ) {
          certificateFile = boost::filesystem::path (
                              config.get<std::string> ("configPath") ) / certificateFile;
        }

        if (!boost::filesystem::exists (certificateFile) ) {
          throw configuration_exception ("Cannot get cerfificate file for secure websocket, configured file "
                                         + certificateFile.string() + " does not exist");
        }
      } catch (const boost::property_tree::ptree_error &err) {
        throw configuration_exception ("Cannot get cerfificate file for secure websocket");
      }

      secureServer.clear_access_channels (websocketpp::log::alevel::all);
      secureServer.clear_error_channels (websocketpp::log::alevel::all);

      secureServer.init_asio (&ios);
      secureServer.set_reuse_addr (true);

      secureServer.set_open_handler (std::bind ( (void (WebSocketTransport::*) (
                                       SecureWebSocketServer *,
                                       websocketpp::connection_hdl) ) &WebSocketTransport::openHandler, this,
                                     &secureServer, std::placeholders::_1) );
      secureServer.set_close_handler (std::bind (&WebSocketTransport::closeHandler,
                                      this,
                                      std::placeholders::_1) );
      secureServer.set_message_handler (std::bind ( (void (WebSocketTransport::*) (
                                          SecureWebSocketServer *, websocketpp::connection_hdl,
                                          SecureWebSocketServer::message_ptr) ) &WebSocketTransport::processMessage,
                                        this, &secureServer, std::placeholders::_1,
                                        std::placeholders::_2) );

      secureServer.set_tls_init_handler ( [password, certificateFile] (
      websocketpp::connection_hdl hdl) -> context_ptr {
        context_ptr context (new boost::asio::ssl::context (boost::asio::ssl::context::tlsv1) );

        try {
          context->set_options (boost::asio::ssl::context::default_workarounds |
          boost::asio::ssl::context::no_sslv2 |
          boost::asio::ssl::context::single_dh_use);
          context->set_password_callback(std::bind([password]() -> std::string {
            GST_INFO ("password");
            return password;
          }));
          context->use_certificate_chain_file (certificateFile.string() );
          context->use_private_key_file (certificateFile.string(), boost::asio::ssl::context::pem);
        } catch (std::exception &e)
        {
          GST_ERROR ("Error while setting up tls %s", e.what() );
        }

        return context;
      });

      try {
        if (ipv6) {
          secureServer.listen (boost::asio::ip::tcp::v6(), securePort);
        }
        else {
          secureServer.listen (boost::asio::ip::tcp::v4(), securePort);
        }
        hasSecureServer = true;
      } catch (websocketpp::exception &e) {
        throw configuration_exception ("Error listening on port " +
                                       std::to_string (securePort) );
      }

      ep = secureServer.get_local_endpoint (ep_err);
      if (!ep_err) {
        GST_INFO ("Secure WebSocket server listening on address '%s', port %u",
            ep.address().to_string().c_str(), ep.port());
      }

    } catch (const configuration_exception &err) {
      GST_WARNING ("Secure websocket server not enabled: %s", err.what() );
    }
  } else {
    GST_INFO ("Secure websocket server not enabled");
  }

  registrarAddress =
    config.get<std::string> ("mediaServer.net.websocket.registrar.address", "");
  localAddress =
    config.get<std::string> ("mediaServer.net.websocket.registrar.localAddress",
                             DEFAULT_LOCAL_ADDRESS);

  if (!registrarAddress.empty () && !localAddress.empty () ) {
    registrar = std::make_shared<WebSocketRegistrar>(
        registrarAddress, localAddress, port, securePort, path);
  }
}

WebSocketTransport::~WebSocketTransport() noexcept = default;

void WebSocketTransport::run()
{
  bool running = true;

  while (running) {
    try {
      ios.run();
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
  std::unique_lock<std::recursive_mutex> lock (mutex);

  while (isRunning() ) {
    std::list<std::string> conns;

    for (auto c : connections) {
      conns.push_back (c.first);
    }

    lock.unlock ();

    for (auto c : conns) {
      GST_INFO ("Keep alive %s", c.c_str() );

      try {
        processor->keepAliveSession (c);
      } catch (KurentoException &e) {
        if (e.getCode() == INVALID_SESSION) {
          GST_INFO ("Session should be removed %s", c.c_str() );
          // TODO: Remove session
        } else {
          throw e;
        }
      }
    }

    lock.lock();
    cond.wait_for (lock, MediaSet::getCollectorInterval() / 4);
  }
}

void WebSocketTransport::start ()
{
  server.start_accept();

  if (hasSecureServer) {
    secureServer.start_accept();
  }

  for (int i = 0; i < n_threads; i++) {
    threads.emplace_back(std::bind(&WebSocketTransport::run, this));
  }

  std::unique_lock<std::recursive_mutex> lock (mutex);
  running = true;
  keepAliveThread = std::thread (std::bind (
                                   &WebSocketTransport::keepAliveSessions, this) );

  if (registrar) {
    registrar->start();
  }
}

void WebSocketTransport::stop ()
{
  std::unique_lock<std::recursive_mutex> lock (mutex);
  running = false;
  cond.notify_all();
  lock.unlock();

  GST_DEBUG ("stop transport");
  server.stop();

  for (int i = 0; i < n_threads; i++) {
    threads[i].join();
  }

  if (registrar) {
    registrar->stop();
  }

  keepAliveThread.join();
}

websocketpp::connection_hdl
WebSocketTransport::getConnection (const std::string &sessionId)
{
  try {
    std::unique_lock<std::recursive_mutex> lock (mutex);
    return connections.at (sessionId);
  } catch (std::out_of_range &e) {
    throw std::out_of_range ("Connection not found for sessionId: " + sessionId);
  }
}

void WebSocketTransport::storeConnection (const std::string &request,
    const std::string &response, websocketpp::connection_hdl connection,
    bool secure, std::string &sessionId)
{
  if (!sessionId.empty() ) {
    std::unique_lock<std::recursive_mutex> lock (mutex);
    bool needsWrite = false;

    try {
      websocketpp::connection_hdl conn = connections.at (sessionId);

      if (conn.lock() != connection.lock() ) {
        GST_WARNING ("Erasing old connection associated with: %s",
                     sessionId.c_str() );
        connectionsReverse.erase (conn);
        connections.erase (sessionId);
        secureConnections.erase (sessionId);
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

      try {
        processor->keepAliveSession (sessionId);
      } catch (KurentoException &e) {
        if (e.getCode () != INVALID_SESSION) {
          throw e;
        }
      }
    }

    secureConnections[sessionId] = secure;
  }
}

void
WebSocketTransport::send (const std::string &sessionId,
                          const std::string &message)
{
  std::unique_lock <std::recursive_mutex> lock (mutex);
  websocketpp::connection_hdl hdl = getConnection (sessionId);

  try {
    if (secureConnections[sessionId]) {
      lock.unlock();
      secureServer.send (hdl, message, websocketpp::frame::opcode::TEXT);
    } else {
      lock.unlock();
      server.send (hdl, message, websocketpp::frame::opcode::TEXT);
    }
  } catch (std::exception &e) {
    GST_ERROR ("Error sending event: %s", e.what() );
  }
}

template <typename ServerType>
void WebSocketTransport::processMessage (ServerType *s,
    websocketpp::connection_hdl hdl, typename ServerType::message_ptr msg)
{
  std::string request = msg->get_payload();
  std::string response;
  std::string sessionId;

  try {
    sessionId = connectionsReverse.at (hdl);
  } catch (std::out_of_range &e) {
    /* Ignore, there is no previous sessionId */
  }

  GST_DEBUG ("Message: %s", request.c_str() );
  sessionId = processor->process (request, response, sessionId);
  GST_DEBUG ("Response: %s", response.c_str() );

  storeConnection (request, response, hdl,
                   std::is_same<ServerType, SecureWebSocketServer>::value, sessionId);

  try {
    s->send (hdl, response, websocketpp::frame::opcode::TEXT);
  } catch (websocketpp::exception &e) {
    GST_ERROR ("Could not send response to client: %s",
               e.code().message().c_str() );
  }
}

template <typename ServerType>
void WebSocketTransport::openHandler (ServerType *s,
                                      websocketpp::connection_hdl hdl)
{
  auto connection = s->get_con_from_hdl (hdl);
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
      s->close (hdl, websocketpp::close::status::protocol_error, "Invalid path");
    } catch (websocketpp::exception &e) {
      GST_ERROR ("Error: %s", e.code().message().c_str() );
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
  std::string eventId = sessionId + "|" + obj->getId() + "|" + eventType;
  std::shared_ptr <EventHandler> handler;
  std::unique_lock<std::recursive_mutex> lock (mutex);

  if (handlers.find (eventId) != handlers.end() ) {
    handler = handlers[eventId].lock();
  }

  if (!handler) {
    handler = std::shared_ptr <EventHandler> (new WebSocketEventHandler (obj,
              shared_from_this(), sessionId) );

    subscriptionId = processor->connectEventHandler (obj, sessionId, eventType,
                     handler);
    handlers[eventId] = std::weak_ptr <EventHandler> (handler);
  } else {
    subscriptionId = generateUUID();
    processor->registerEventHandler (obj, sessionId, subscriptionId, handler);
  }

  return subscriptionId;
}

void WebSocketTransport::closeHandler (websocketpp::connection_hdl hdl)
{
  GST_DEBUG ("Connection closed");

  try {
    std::unique_lock<std::recursive_mutex> lock (mutex);
    std::string sessionId = connectionsReverse.at (hdl);

    GST_DEBUG ("Erasing connection associated with: %s", sessionId.c_str() );
    connections.erase (sessionId);
    connectionsReverse.erase (hdl);
    secureConnections.erase (sessionId);
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
