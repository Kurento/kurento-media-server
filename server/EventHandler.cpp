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

#include "EventHandler.hpp"
#include <utils/utils.hpp>
#include "thrift/transport/TSocket.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include <JsonRpcConstants.hpp>
#include <gst/gst.h>
#include "common/MediaSet.hpp"

#include "KmsMediaHandlerService.h"

#define GST_CAT_DEFAULT kurento_event_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoEventHandler"

using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::protocol;

namespace kurento
{

EventHandler::EventHandler (const std::string &sessionId,
                            const std::string &objectId, const std::string &ip,
                            int port) :
  ip (ip), port (port), sessionId (sessionId), objectId (objectId)
{
  generateUUID (id);
}

EventHandler::~EventHandler()
{
  GST_INFO ("Disconnect event handler %s", id.c_str() );

  try {
    MediaSet::getMediaSet().getMediaObject (objectId);

    conn.disconnect();
  } catch (...) {
    GST_ERROR ("Object %s already released", objectId.c_str() );
  }
}

void
EventHandler::sendEvent (Json::Value &value) const
{
  std::shared_ptr <const EventHandler> handler = shared_from_this();

  pool.push ([handler, value] () {
    boost::shared_ptr<TSocket> socket (new TSocket (handler->ip, handler->port) );
    boost::shared_ptr<TTransport> transport (new TFramedTransport (socket) );
    boost::shared_ptr<TBinaryProtocol> protocol (new TBinaryProtocol (transport) );
    KmsMediaHandlerServiceClient client (protocol);

    try {
      Json::FastWriter writer;
      Json::Value rpc;
      Json::Value event;

      transport->open();

      event ["value"] = value;
      event ["sessionId"] = handler->sessionId;

      rpc [JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;
      rpc [JSON_RPC_METHOD] = "onEvent";
      rpc [JSON_RPC_PARAMS] = event;

      GST_DEBUG ("Sending event: %s", writer.write (rpc).c_str() );
      client.eventJsonRpc (writer.write (rpc) );

      transport->close();
    } catch (std::exception &e) {
      GST_WARNING ("Error sending event to MediaHandler(%s, %s:%d)",
                   handler->id.c_str(), handler->ip.c_str(), handler->port);
    } catch (...) {
      GST_WARNING ("Error sending event to MediaHandler(%s, %s:%d)",
                   handler->id.c_str(), handler->ip.c_str(), handler->port);
    }
  });
}

Glib::ThreadPool EventHandler::pool;

EventHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

EventHandler::StaticConstructor EventHandler::staticConstructor;


} /* kurento */
