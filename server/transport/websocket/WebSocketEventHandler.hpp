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
