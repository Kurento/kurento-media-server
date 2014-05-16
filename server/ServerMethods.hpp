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

#ifndef __SERVER_METHODS__
#define __SERVER_METHODS__

#include <JsonRpcHandler.hpp>
#include <EventHandler.hpp>

namespace kurento
{

class MediaObject;

class ServerMethods
{

public:
  ServerMethods();
  virtual ~ServerMethods() {};

  void process (const std::string &request, std::string &response);

protected:

  virtual std::string connectEventHandler (std::shared_ptr<MediaObject> obj,
      const std::string &sessionId, const std::string &eventType,
      const Json::Value &params) = 0;

  std::string connectEventHandler (std::shared_ptr<MediaObject> obj,
                                   const std::string &sessioId, const std::string &eventType,
                                   std::shared_ptr<EventHandler> handler);

  void registerEventHandler (const std::string &sessionId,
                             const  std::string &subscriptionId,
                             std::shared_ptr<EventHandler> handler);

private:

  void create (const Json::Value &params, Json::Value &response);
  void invoke (const Json::Value &params, Json::Value &response);
  void subscribe (const Json::Value &params, Json::Value &response);
  void unsubscribe (const Json::Value &params, Json::Value &response);
  void release (const Json::Value &params, Json::Value &response);
  void ref (const Json::Value &params, Json::Value &response);
  void unref (const Json::Value &params, Json::Value &response);
  void keepAlive (const Json::Value &params, Json::Value &response);
  void describe (const Json::Value &params, Json::Value &response);

  JsonRpc::Handler handler;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __SERVER_METHODS__ */
