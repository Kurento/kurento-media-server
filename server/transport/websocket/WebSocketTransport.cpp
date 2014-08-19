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

#define GST_CAT_DEFAULT kurento_websocket_transport
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebSocketTransport"

namespace kurento
{

/* Default config values */
const std::string WEBSOCKET_ADDRESS_DEFAULT = "127.0.0.1";
const uint WEBSOCKET_PORT_DEFAULT = 9090;
const std::string WEBSOCKET_PATH_DEFAULT = "kurento";

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
  std::string address;
  std::string path;
  uint port;

  try {
    address = config.get<std::string> ("mediaServer.net.websocket.address");
  } catch (const boost::property_tree::ptree_error &err) {
    GST_WARNING ("Setting default address %s to websocket",
                 WEBSOCKET_ADDRESS_DEFAULT.c_str() );
    address = WEBSOCKET_ADDRESS_DEFAULT;
  }

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
}

WebSocketTransport::~WebSocketTransport()
{
}

void WebSocketTransport::start ()
{
}

void WebSocketTransport::stop ()
{
  GST_DEBUG ("stop transport");
}

WebSocketTransport::StaticConstructor WebSocketTransport::staticConstructor;

WebSocketTransport::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
