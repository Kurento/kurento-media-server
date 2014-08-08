/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
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

#include "ThriftServiceHandler.hpp"
#include "KmsMediaServer_constants.h"
#include <ThriftEventHandler.hpp>
#include <jsonrpc/JsonRpcUtils.hpp>
#include <gst/gst.h>
#include <sys/socket.h>

#define GST_CAT_DEFAULT kurento_media_server_service_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoThriftServiceHandler"

namespace kurento
{

ThriftServiceHandler::ThriftServiceHandler (const boost::property_tree::ptree
    &config) :
  ServerMethods (config)
{
}

ThriftServiceHandler::~ThriftServiceHandler ()
{
}

void
ThriftServiceHandler::invokeJsonRpc (std::string &_return,
                                     const std::string &request)
{
  GST_DEBUG ("Json request: %s", request.c_str() );
  process (request, _return);
  GST_DEBUG ("Json response: %s", _return.c_str() );
}

std::string
ThriftServiceHandler::connectEventHandler (std::shared_ptr<MediaObjectImpl>
    obj, const std::string &sessionId, const std::string &eventType,
    const Json::Value &params)
{
  std::shared_ptr <EventHandler> handler;
  std::string id;
  std::string ip;
  int port;

  JsonRpc::getValue (params, "ip", ip);
  JsonRpc::getValue (params, "port", port);

  handler = std::shared_ptr <EventHandler> (new ThriftEventHandler (obj,
            sessionId, ip, port) );

  id = ServerMethods::connectEventHandler (obj, sessionId, eventType, handler);

  std::dynamic_pointer_cast<ThriftEventHandler> (handler)->setId (id);

  return id;
}


ThriftServiceHandler::StaticConstructor
ThriftServiceHandler::staticConstructor;

ThriftServiceHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
