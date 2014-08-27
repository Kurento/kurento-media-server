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
#include "WebSocketEventHandler.hpp"
#include <jsonrpc/JsonRpcConstants.hpp>

#define GST_CAT_DEFAULT kurento_websocket_event_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebSocketEventHandler"

namespace kurento
{

WebSocketEventHandler::WebSocketEventHandler (std::shared_ptr <MediaObjectImpl>
    object, std::shared_ptr<WebSocketTransport> transport,
    std::string sessionId) : EventHandler (object), transport (transport),
  sessionId (sessionId)
{

}

void
WebSocketEventHandler::sendEvent (Json::Value &value)
{
  try {
    Json::FastWriter writer;
    Json::Value rpc;
    Json::Value event;
    std::string eventStr;

    event ["value"] = value;

    rpc [JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;
    rpc [JSON_RPC_METHOD] = "onEvent";
    rpc [JSON_RPC_PARAMS] = event;

    eventStr = writer.write (rpc);
    GST_DEBUG ("Sending event: %s -> %s", eventStr.c_str(),
               sessionId.c_str() );

    try {
      transport->send (sessionId, eventStr);
    } catch (websocketpp::lib::error_code &e) {
      GST_ERROR ("Error on websocket while sending event to MediaHandler: %s",
                 e.message().c_str() );
    }
  } catch (std::exception &e) {
    GST_WARNING ("Error sending event to MediaHandler: %s", e.what() );
  } catch (...) {
    GST_WARNING ("Error sending event to MediaHandler");
  }
}

WebSocketEventHandler::StaticConstructor
WebSocketEventHandler::staticConstructor;

WebSocketEventHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
