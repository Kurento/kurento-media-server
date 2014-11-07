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

#include "BaseTest.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <thread>

#define BOOST_TEST_MODULE ServerTest
#include <boost/test/unit_test.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <sys/types.h>
#include <sys/wait.h>
#include <gst/gst.h>

#define GST_CAT_DEFAULT _base_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_base"

namespace kurento
{

static const std::chrono::seconds REPLY_TIMEOUT (10);
static const int MAX_RETRIES = 20;
static const std::string WS_PATH = "/kurento";
static const std::string WS_PROTO = "ws://";
static const std::string WS_ADDRESS = "localhost";

static const std::string SDP_PATTERN = "sdp_pattern.txt";

void F::create_ws_uri (uint port)
{
  uri = WS_PROTO + WS_ADDRESS + ":" + std::to_string (port) + WS_PATH;
}

boost::filesystem::path
F::write_config (const boost::filesystem::path &orig, uint port)
{
  configDir = boost::filesystem::unique_path (
                boost::filesystem::temp_directory_path() / "kms_config_%%%%%%%%" );
  boost::filesystem::create_directories (configDir);
  boost::filesystem::path newConfigFile = configDir / "config.json";

  // Copy sdp pattern
  boost::filesystem::copy_file (orig.parent_path() / SDP_PATTERN,
                                configDir / SDP_PATTERN);

  // Change port on config
  boost::property_tree::ptree config;
  boost::property_tree::json_parser::read_json (orig.string(), config);
  boost::property_tree::ptree &wsConfig =
    config.get_child ("mediaServer.net.websocket");
  wsConfig.erase ("port");
  wsConfig.add ("port", port);

  boost::property_tree::json_parser::write_json (newConfigFile.string(), config);

  return newConfigFile;
}

void
F::start_server ()
{
  pid = -1;
  char *conf_file, *binary_dir;

  conf_file = getenv ("MEDIA_SERVER_CONF_FILE");
  binary_dir = getenv ("SERVER_DIR");

  if (conf_file == NULL) {
    BOOST_FAIL ("No configuration file for mediaserver");
  }

  // Find an empty port
  boost::asio::io_service ios;
  boost::asio::ip::tcp::endpoint ep (boost::asio::ip::tcp::v6(), 0);
  boost::asio::ip::tcp::acceptor acceptor (ios, ep);
  acceptor.set_option (boost::asio::socket_base::reuse_address (true) );
  acceptor.listen();
  uint port = acceptor.local_endpoint().port();

  boost::filesystem::path configFile = write_config (boost::filesystem::path (
                                         conf_file), port);
  create_ws_uri (port);

  GST_DEBUG ("Binding to port: %d", port);
  acceptor.close();

  pid = fork();

  if (pid == 0) {
    std::string  confFileParam = "--conf-file=" + configFile.string();

    execl (binary_dir, "kurento-media-server", confFileParam.c_str(), NULL);
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

  BOOST_REQUIRE_MESSAGE (sendingMessage, "Unexpected message received");
  receivedMessage = true;
  response = msg->get_payload();
  cond.notify_all();
}

std::string F::sendMessage (const std::string &message)
{
  std::string response;
  std::unique_lock <std::mutex> lock (mutex);

  BOOST_REQUIRE_MESSAGE (!sendingMessage, "Already sending a message");

  sendingMessage = true;
  client->send (connectionHdl, message, websocketpp::frame::opcode::text);

  if (!cond.wait_for (lock, REPLY_TIMEOUT, std::bind (&F::received,
                      this) ) ) {
    BOOST_FAIL ("Timeout wainting for response");
  }

  sendingMessage = false;
  receivedMessage = false;
  response.swap (this->response);

  return response;
}

void F::start_client()
{
  std::unique_lock <std::mutex> lock (mutex);

  while (!initialized && !terminate) {
    client = std::shared_ptr <WebSocketClient> (new WebSocketClient() );

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
  bool wasLocked;

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

F::F ()
{
  int retries = 0;

  id = 0;

  gst_init (NULL, NULL);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  start_server();
  BOOST_REQUIRE_MESSAGE (pid > 0, "Error launching mediaserver");

  clientThread = std::thread (std::bind (&F::start_client, this) );

  std::unique_lock <std::mutex> lock (mutex);

  while (!initialized && retries < MAX_RETRIES) {
    lock.unlock();
    std::this_thread::sleep_for (std::chrono::milliseconds (100 * (retries + 1) ) );
    lock.lock();

    if (!initialized) {
      GST_INFO ("Waiting, %d times", retries);
      retries++;
    }
  }

  if (!initialized) {
    stop_client (lock);
    lock.unlock();

    stop_server ();
  }

  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
}

F::~F()
{
  GST_DEBUG ("teardown fixture");
  std::unique_lock <std::mutex> lock (mutex);

  if (initialized) {
    stop_client (lock);
  }

  lock.unlock();

  stop_server ();
}

} /* kurento */
