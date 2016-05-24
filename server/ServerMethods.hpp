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

  virtual std::string process (const std::string &request, std::string &response,
                               std::string &sessionId);

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
  void ping (const Json::Value &params, Json::Value &response);
  void closeSession (const Json::Value &params, Json::Value &response);

  const boost::property_tree::ptree &config;
  JsonRpc::Handler handler;

  float resourceLimitPercent;

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
