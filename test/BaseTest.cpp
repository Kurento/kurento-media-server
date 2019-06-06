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

#include "BaseTest.hpp"

#include "KurentoException.hpp"

#include <memory>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <thread>

#include <jsonrpc/JsonRpcConstants.hpp>

#define BOOST_TEST_MODULE ServerTest
#include <boost/test/unit_test.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <sys/types.h>
#include <sys/wait.h>
#include <gst/gst.h>

#include <errno.h>
#include <string.h>

#define GST_CAT_DEFAULT _base_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_base"

namespace kurento
{

static const std::chrono::seconds REPLY_TIMEOUT (10);
static const int MAX_RETRIES = 20;
static const std::string WS_PATH = "/kurento";
static const std::string WS_PROTO = "ws://";

void F::create_ws_uri (uint port)
{
  uri = WS_PROTO + wsHost + ":" + std::to_string (port) + WS_PATH;
}

boost::filesystem::path
F::write_config (const boost::filesystem::path &orig, uint port)
{
  configDir = boost::filesystem::unique_path (
                boost::filesystem::temp_directory_path() / "kms_config_%%%%%%%%" );
  boost::filesystem::create_directories (configDir);
  boost::filesystem::path newConfigFile = configDir / "config.conf.json";


  // Change port on config
  boost::property_tree::ptree config;
  boost::property_tree::json_parser::read_json (orig.string(), config);

  boost::property_tree::ptree &wsConfig =
    config.get_child ("mediaServer.net.websocket");
  wsConfig.erase ("port");
  wsConfig.add ("port", port);

  boost::property_tree::ptree &resourceConfig =
    config.get_child ("mediaServer.resources");

  resourceConfig.erase ("exceptionLimit");
  resourceConfig.add ("exceptionLimit", resourceLimit);

  boost::property_tree::json_parser::write_json (newConfigFile.string(), config);

  return newConfigFile;
}

void
F::start_server ()
{
  pid = -1;
  char *conf_path, *bin_path;

  bin_path = getenv ("TEST_BIN_PATH");
  if (bin_path == nullptr) {
    BOOST_FAIL ("TEST_BIN_PATH is unset");
  }

  conf_path = getenv ("TEST_CONF_PATH");
  if (conf_path == nullptr) {
    BOOST_FAIL ("TEST_CONF_PATH is unset");
  }

  GST_DEBUG ("Use TEST_BIN_PATH=%s", bin_path);
  GST_DEBUG ("Use TEST_CONF_PATH=%s", conf_path);

  // Find an empty port
  boost::asio::io_service ios;
  boost::asio::ip::tcp::endpoint ep (boost::asio::ip::tcp::v6(), 0);
  boost::asio::ip::tcp::acceptor acceptor (ios, ep);
  acceptor.set_option (boost::asio::socket_base::reuse_address (true) );
  acceptor.listen();
  uint port = acceptor.local_endpoint().port();

  boost::filesystem::path configFilePath = boost::filesystem::absolute
      (write_config (boost::filesystem::path (conf_path), port));

  create_ws_uri (port);

  GST_DEBUG ("Binding to port: %d", port);
  acceptor.close();


  pid = fork();

  if (pid == 0) {
    int rc = execl (bin_path, bin_path,
        //"--modules-path=.",
        //"--modules-config-path=./config",
        //"--conf-file=./config/kurento.conf.json",
        std::string("--conf-file=" + configFilePath.native()).c_str(),
        //"--gst-plugin-path=.",
        NULL
    );
    if (rc == -1) {
      BOOST_FAIL ("Error in execl(): " << strerror(errno)
          << " (" << bin_path << ")");
    }
  } else if (pid < 0) {
    BOOST_FAIL ("Error executing server");
  }
}

void
F::stop_server ()
{
  int status;

  kill (pid, SIGINT);

  if (pid != waitpid (pid, &status, 0) ) {
    BOOST_FAIL ("Error waiting for child process");
  }

  pid = -1;

  boost::filesystem::remove_all (configDir);
}

void F::on_open (websocketpp::connection_hdl hdl)
{
  std::unique_lock <std::mutex> lock (mutex);
  GST_DEBUG ("connection ready");
  connectionHdl = hdl;
  initialized = true;
}

void F::on_message (websocketpp::connection_hdl hdl,
                    WebSocketClient::message_ptr msg)
{
  std::unique_lock <std::mutex> lock (mutex);
  GST_DEBUG ("Received a reply: %s", msg->get_payload().c_str() );

  Json::Value recvMessage;
  BOOST_REQUIRE_MESSAGE (reader.parse (msg->get_payload(), recvMessage),
                         "Cannot parse input message");
  recvMessages.push_back (recvMessage);

  cond.notify_all();
}

Json::Value F::getResponse (const std::string &requestId)
{
  for (auto it = recvMessages.begin(); it != recvMessages.end(); it++) {
    Json::Value message = *it;

    if (isResponse (message, requestId) ) {
      recvMessages.erase (it);
      return message;
    }
  }

  throw KurentoException (UNEXPECTED_ERROR, "Resonse not found");
}

Json::Value F::getFirstEvent ()
{
  for (auto it = recvMessages.begin(); it != recvMessages.end(); it++) {
    Json::Value message = *it;

    if (isEvent (message) ) {
      recvMessages.erase (it);
      return message;
    }
  }

  throw KurentoException (UNEXPECTED_ERROR, "Resonse not found");
}

Json::Value F::sendRequest (const Json::Value &request)
{
  std::string response_str, requestId;
  Json::Value response;
  std::unique_lock <std::mutex> lock (mutex);

  BOOST_REQUIRE_MESSAGE (initialized, "Not initialized");
  BOOST_REQUIRE_MESSAGE (!sendingMessage, "Already sending a message");

  sendingMessage = true;

  Json::StreamWriterBuilder writerFactory;
  writerFactory["indentation"] = "";
  client->send (connectionHdl, Json::writeString (writerFactory, request),
                websocketpp::frame::opcode::text);

  requestId = request[JSON_RPC_ID].asString();

  if (!cond.wait_for (lock, REPLY_TIMEOUT, std::bind (&F::receivedResponse,
                      this, requestId ) ) ) {
    BOOST_FAIL ("Timeout waiting for response");
  }

  sendingMessage = false;

  response = getResponse (requestId);

  return response;
}

Json::Value F::waifForEvent (const std::chrono::seconds timeout)
{
  Json::Value event;
  std::unique_lock <std::mutex> lock (mutex);

  try {
    event = getFirstEvent();
  } catch (KurentoException e) {
    if (!cond.wait_for (lock, timeout, std::bind (&F::receivedEvent, this) ) ) {
      throw kurento::KurentoException (UNEXPECTED_ERROR, "Timeout waiting for event");
    }

    event = getFirstEvent();
  }

  return event;
}

void F::start_client()
{
  std::unique_lock <std::mutex> lock (mutex);

  while (!initialized && !terminate) {
    client = std::make_shared<WebSocketClient>();

    client->clear_access_channels (websocketpp::log::alevel::all);
    client->clear_error_channels (websocketpp::log::elevel::all);

    // Register our handlers
    client->set_open_handler (std::bind (&F::on_open, this,
                                         std::placeholders::_1) );
    client->set_message_handler (std::bind (&F::on_message, this,
                                            std::placeholders::_1, std::placeholders::_2) );

    // Initialize ASIO
    client->init_asio();

    // We expect there to be a lot of errors, so suppress them
    websocketpp::lib::error_code ec;
    WebSocketClient::connection_ptr con = client->get_connection (uri, ec);

    if (ec) {
      BOOST_ERROR (ec.message() );
    }

    client->connect (con);

    // Start the ASIO io_service run loop
    mutex.unlock();
    client->run();

    if (!initialized) {
      std::this_thread::sleep_for (std::chrono::milliseconds (50) );
    }

    mutex.lock();
  }
}

void F::stop_client (std::unique_lock <std::mutex> &lock)
{
  bool wasLocked = false;

  if (lock) {
    wasLocked = true;
  }

  if (!wasLocked) {
    lock.lock();
  }

  if (client) {
    client->stop();
  }

  terminate = true;

  lock.unlock();

  GST_DEBUG ("Waiting for client thread to finish");
  clientThread.join();

  if (wasLocked) {
    lock.lock();
  }
}

bool F::isEvent (const Json::Value &message)
{
  if (message.isMember (JSON_RPC_METHOD)
      && message[JSON_RPC_METHOD] == "onEvent") {
    return true;
  }

  return false;
}

bool F::isResponse (const Json::Value &message, const std::string &requestId)
{
  if (message.isMember (JSON_RPC_ERROR)
      || (message.isMember (JSON_RPC_RESULT)
          && message.isMember (JSON_RPC_ID)
          && message[JSON_RPC_ID].asString() == requestId) ) {
    return true;
  }

  return false;
}

bool F::receivedResponse (const std::string &requestId)
{
  for (Json::Value message : recvMessages) {
    if (isResponse (message, requestId) ) {
      return true;
    }
  }

  return false;
}

bool F::receivedEvent ()
{
  for (Json::Value message : recvMessages) {
    if (isEvent (message) ) {
      return true;
    }
  }

  return false;
}

void
F::start ()
{
  int retries = 0;

  std::unique_lock <std::mutex> lock (mutex);

  id = 0;

  start_server();

  BOOST_REQUIRE_MESSAGE (pid > 0, "Error launching Kurento Media Server");

  clientThread = std::thread (std::bind (&F::start_client, this) );

  while (!initialized && retries < MAX_RETRIES) {
    lock.unlock();
    std::this_thread::sleep_for (std::chrono::milliseconds (100 * (retries + 1) ) );
    lock.lock();

    if (!initialized) {
      GST_INFO ("Waiting, %d times", retries);
      retries++;
    }
    else {
      GST_WARNING ("Initialized");
    }
  }

  if (!initialized) {
    stop_client (lock);
    lock.unlock();

    stop_server ();
  }

  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
}

void
F::stop()
{
  std::unique_lock <std::mutex> lock (mutex);

  if (initialized) {
    stop_client (lock);
  }

  initialized = false;

  lock.unlock();

  stop_server ();
}

F::F ()
{
  gst_init(nullptr, nullptr);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

F::~F()
{
  GST_DEBUG ("teardown fixture");

  if (initialized) {
    stop();
  }
}

} /* kurento */
