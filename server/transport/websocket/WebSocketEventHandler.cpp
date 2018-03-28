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

#include "WebSocketEventHandler.hpp"

#include <gst/gst.h>
#include <json/json.h>
#include <jsonrpc/JsonRpcConstants.hpp>
#include <utility>

#define GST_CAT_DEFAULT kurento_websocket_event_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebSocketEventHandler"

namespace kurento
{

WebSocketEventHandler::WebSocketEventHandler(
    std::shared_ptr<MediaObjectImpl> object,
    std::shared_ptr<WebSocketTransport> transport, std::string sessionId)
    : EventHandler(object), transport(std::move(transport)),
      sessionId(std::move(sessionId)) {}

void
WebSocketEventHandler::sendEvent (Json::Value &value)
{
  try {
    Json::Value rpc;
    Json::Value event;
    std::string eventStr;

    event ["value"] = value;

    rpc [JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;
    rpc [JSON_RPC_METHOD] = "onEvent";
    rpc [JSON_RPC_PARAMS] = event;

    Json::StreamWriterBuilder writerFactory;
    writerFactory["indentation"] = "";
    eventStr = Json::writeString (writerFactory, rpc);
    GST_DEBUG ("Sending event: %s, sessionId: %s", eventStr.c_str(),
               sessionId.c_str() );

    try {
      transport->send (sessionId, eventStr);
    } catch (websocketpp::exception &e) {
      GST_ERROR ("Error on websocket while sending event to MediaHandler: %s",
                 e.code().message().c_str() );
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
