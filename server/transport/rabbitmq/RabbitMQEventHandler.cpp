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

#include "RabbitMQEventHandler.hpp"
#include <jsonrpc/JsonRpcConstants.hpp>
#include <gst/gst.h>

#define GST_CAT_DEFAULT kurento_rabbitmq_event_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRabbitMQEventHandler"

namespace kurento
{

RabbitMQEventHandler::RabbitMQEventHandler (std::shared_ptr< MediaObjectImpl >
    obj,
    const std::string &address, int port,
    const std::string &exchange, const std::string &routingKey) :
  EventHandler (obj), connection (address, port), exchange (exchange),
  routingKey (routingKey)
{
}

RabbitMQEventHandler::~RabbitMQEventHandler()
{
}

void
RabbitMQEventHandler::sendEvent (Json::Value &value)
{
  try {
    Json::FastWriter writer;
    Json::Value rpc;
    Json::Value event;

    event ["value"] = value;

    rpc [JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;
    rpc [JSON_RPC_METHOD] = "onEvent";
    rpc [JSON_RPC_PARAMS] = event;

    GST_DEBUG ("Sending event: %s -> %s", writer.write (rpc).c_str(),
               exchange.c_str() );
    connection.sendMessage (writer.write (rpc), exchange, routingKey);
  } catch (std::exception &e) {
    GST_WARNING ("Error sending event to MediaHandler");
  } catch (...) {
    GST_WARNING ("Error sending event to MediaHandler");
  }
}

RabbitMQEventHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

RabbitMQEventHandler::StaticConstructor RabbitMQEventHandler::staticConstructor;

} /* kurento */
