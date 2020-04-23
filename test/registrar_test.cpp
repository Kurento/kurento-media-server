/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
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

#include <config.h>

// #define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ConectRegistrar
#include <boost/test/unit_test.hpp>

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

#include <WebSocketRegistrar.hpp>
#include <boost/filesystem.hpp>

#include <gst/gst.h>
#include <string>

static const int PORT = 9898;
static const int LOCAL_PORT = 1234;
static const std::string LOCAL_ADDRESS = "localhost";
//#define LOCAL_PORT 1234
//#define xstr(s) str(s)
//#define str(s) #s
//#define LOCAL_PORT_STR xstr(LOCAL_PORT)
static const std::string WS_PATH = "path";
static const std::string REGISTRAR_MESSAGE =
    R"({"jsonrpc":"2.0","method":"register","params":{"ws":"ws://)" +
    LOCAL_ADDRESS + ":" + std::to_string(LOCAL_PORT) + "/" + WS_PATH + "\"}}";

static const std::string PASSWORD = "";
static const boost::filesystem::path
CERTIFICATE_FILE (TEST_DIRECTORY "/testCertificate.pem");

typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;
typedef websocketpp::server<websocketpp::config::asio_tls>
SecureWebSocketServer;

typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

BOOST_AUTO_TEST_CASE ( ws_registrar )
{
  WebSocketServer server;
  boost::asio::io_service ios;

  kurento::WebSocketRegistrar registrar ("ws://localhost:" + std::to_string (
      PORT), LOCAL_ADDRESS, LOCAL_PORT, 0, WS_PATH);

  registrar.start();

  // Create websocket
  server.clear_access_channels (websocketpp::log::alevel::all);
  server.clear_error_channels (websocketpp::log::alevel::all);

  server.init_asio (&ios);
  server.set_reuse_addr (true);
  server.set_message_handler ( [&ios] (websocketpp::connection_hdl hdl,
  typename WebSocketServer::message_ptr msg) {
    BOOST_CHECK_EQUAL (msg->get_payload(), REGISTRAR_MESSAGE);
    ios.stop();
  });


  server.listen (PORT);

  server.start_accept();

  // TODO: Add timeout to not block forever
  ios.run();

  registrar.stop();
  server.stop_listening();
}

BOOST_AUTO_TEST_CASE ( wss_registrar )
{
  SecureWebSocketServer server;
  boost::asio::io_service ios;

  gst_init(nullptr, nullptr);

  kurento::WebSocketRegistrar registrar ("wss://localhost:" + std::to_string (
      PORT), LOCAL_ADDRESS, LOCAL_PORT, 0, WS_PATH);

  registrar.start();

  // Create websocket
  server.clear_access_channels (websocketpp::log::alevel::all);
  server.clear_error_channels (websocketpp::log::alevel::all);

  server.init_asio (&ios);
  server.set_reuse_addr (true);
  server.set_message_handler ( [&ios] (websocketpp::connection_hdl hdl,
  typename SecureWebSocketServer::message_ptr msg) {
    BOOST_CHECK_EQUAL (msg->get_payload(), REGISTRAR_MESSAGE);
    ios.stop();
  });

  server.set_tls_init_handler ( [] (websocketpp::connection_hdl hdl) ->
  context_ptr {
    context_ptr context (new boost::asio::ssl::context (boost::asio::ssl::context::tlsv12) );

    try {
      context->set_options (boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2 |
      boost::asio::ssl::context::single_dh_use);
      context->set_password_callback(
          std::bind([]() -> std::string { return PASSWORD; }));
      context->use_certificate_chain_file (CERTIFICATE_FILE.string() );
      context->use_private_key_file (CERTIFICATE_FILE.string(), boost::asio::ssl::context::pem);
    } catch (std::exception &e)
    {
      BOOST_FAIL ("Error while setting up tls " + std::string (e.what() ) );
    }

    return context;
  });

  server.listen (PORT);

  server.start_accept();

  // TODO: Add timeout to not block forever
  ios.run();

  registrar.stop();
  server.stop_listening();
}
