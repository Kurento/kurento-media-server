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

#include <jsonrpc/JsonRpcHandler.hpp>
#include <EventHandler.hpp>
#include <ModuleManager.hpp>
#include <boost/property_tree/ptree.hpp>
#include <Processor.hpp>
#include "RequestCache.hpp"

namespace kurento
{

class MediaObject;

class ServerMethods : public Processor
{

public:
  ServerMethods (const boost::property_tree::ptree &config);
  virtual ~ServerMethods();

  virtual void process (const std::string &request, std::string &response);

  virtual void keepAliveSession (const std::string &sessionId);

protected:

  virtual std::string connectEventHandler (std::shared_ptr<MediaObjectImpl> obj,
      const std::string &sessionId, const std::string &eventType,
      std::shared_ptr<EventHandler> handler);

  virtual void registerEventHandler (std::shared_ptr<MediaObjectImpl> obj,
                                     const std::string &sessionId, const  std::string &subscriptionId,
                                     std::shared_ptr<EventHandler> handler);

  virtual void setEventSubscriptionHandler (std::function < std::string (
        std::shared_ptr<MediaObjectImpl> obj,
        const std::string &sessionId, const std::string &eventType,
        const Json::Value &params) > e)
  {
    eventSubscriptionHandler = e;
  }

private:

  bool preProcess (const Json::Value &request, Json::Value &response);
  void postProcess (const Json::Value &request, Json::Value &response);

  void connect (const Json::Value &params, Json::Value &response);
  void create (const Json::Value &params, Json::Value &response);
  void invoke (const Json::Value &params, Json::Value &response);
  void subscribe (const Json::Value &params, Json::Value &response);
  void unsubscribe (const Json::Value &params, Json::Value &response);
  void release (const Json::Value &params, Json::Value &response);
  void ref (const Json::Value &params, Json::Value &response);
  void unref (const Json::Value &params, Json::Value &response);
  void keepAlive (const Json::Value &params, Json::Value &response);
  void describe (const Json::Value &params, Json::Value &response);
  void transaction (const Json::Value &params, Json::Value &response);

  const boost::property_tree::ptree &config;
  JsonRpc::Handler handler;

  std::function<std::string (std::shared_ptr<MediaObjectImpl> obj, const std::string &sessionId, const std::string &eventType, const Json::Value &params) >
  eventSubscriptionHandler;

  ModuleManager &moduleManager;
  std::shared_ptr<RequestCache> cache;
  std::string instanceId;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __SERVER_METHODS__ */
