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

#ifndef __WEBSOCKET_EVENT_HANDLER_HPP__
#define __WEBSOCKET_EVENT_HANDLER_HPP__

#include "WebSocketTransport.hpp"

namespace kurento
{

class WebSocketEventHandler : public EventHandler
{
public:
  WebSocketEventHandler (std::shared_ptr <MediaObjectImpl> object,
                         std::shared_ptr<WebSocketTransport> transport, std::string sessionId);
  virtual ~WebSocketEventHandler () {};

  virtual void sendEvent (Json::Value &value);

private:

  std::shared_ptr<WebSocketTransport> transport;
  std::string sessionId;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __WEBSOCKET_EVENT_HANDLER_HPP__ */
