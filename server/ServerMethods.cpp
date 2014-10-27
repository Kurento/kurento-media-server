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
#include "ServerMethods.hpp"
#include <MediaSet.hpp>
#include <string>
#include <EventHandler.hpp>
#include <KurentoException.hpp>
#include <jsonrpc/JsonRpcUtils.hpp>
#include <jsonrpc/JsonRpcConstants.hpp>

#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "modules.hpp"
#include <version.hpp>
#include <ServerInfo.hpp>
#include <ModuleInfo.hpp>
#include <ServerType.hpp>

#define GST_CAT_DEFAULT kurento_server_methods
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoServerMethods"

#define SESSION_ID "sessionId"
#define OBJECT "object"
#define SUBSCRIPTION "subscription"

#define REQUEST_TIMEOUT 20000 /* 20 seconds */

static const std::string KURENTO_MODULES_PATH = "KURENTO_MODULES_PATH";

namespace kurento
{

static std::string
generateUUID ()
{
  std::stringstream ss;
  boost::uuids::uuid uuid = boost::uuids::random_generator() ();

  ss << uuid;

  return ss.str();
}

ServerMethods::ServerMethods (const boost::property_tree::ptree &config) :
  config (config), moduleManager (getModuleManager() )
{
  std::string version (get_version() );
  std::vector<std::shared_ptr<ModuleInfo>> modules;
  std::shared_ptr<ServerType> type (new ServerType (ServerType::KMS) );
  std::vector<std::string> capabilities;
  std::shared_ptr <ServerInfo> serverInfo;
  std::shared_ptr<MediaObjectImpl> serverData;

  instanceId = generateUUID();

  for (auto moduleIt : moduleManager.getModules () ) {
    std::vector<std::string> factories;

    for (auto factIt : moduleIt.second->getFactories() ) {
      factories.push_back (factIt.first);
    }

    modules.push_back (std::shared_ptr<ModuleInfo> (new ModuleInfo (
                         moduleIt.second->getVersion(), moduleIt.second->getName(), factories) ) );
  }

  serverInfo = std::shared_ptr <ServerInfo> (new ServerInfo (version, modules,
               type, capabilities) );

  serverData = MediaSet::getMediaSet ()->ref (new ServerImpl (serverInfo,
               config) );
  MediaSet::getMediaSet ()->setServer (std::dynamic_pointer_cast <ServerImpl>
                                       (serverData) );

  cache = std::shared_ptr<RequestCache> (new RequestCache (REQUEST_TIMEOUT) );

  handler.setPreProcess (std::bind (&ServerMethods::preProcess, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2) );
  handler.setPostProcess (std::bind (&ServerMethods::postProcess, this,
                                     std::placeholders::_1,
                                     std::placeholders::_2) );

  handler.addMethod ("connect", std::bind (&ServerMethods::connect, this,
                     std::placeholders::_1,
                     std::placeholders::_2) );
  handler.addMethod ("create", std::bind (&ServerMethods::create, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2) );
  handler.addMethod ("invoke", std::bind (&ServerMethods::invoke, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2) );
  handler.addMethod ("subscribe", std::bind (&ServerMethods::subscribe, this,
                     std::placeholders::_1,
                     std::placeholders::_2) );
  handler.addMethod ("unsubscribe", std::bind (&ServerMethods::unsubscribe,
                     this, std::placeholders::_1,
                     std::placeholders::_2) );
  handler.addMethod ("release", std::bind (&ServerMethods::release,
                     this, std::placeholders::_1,
                     std::placeholders::_2) );
  handler.addMethod ("ref", std::bind (&ServerMethods::ref,
                                       this, std::placeholders::_1,
                                       std::placeholders::_2) );
  handler.addMethod ("unref", std::bind (&ServerMethods::unref,
                                         this, std::placeholders::_1,
                                         std::placeholders::_2) );
  handler.addMethod ("keepAlive", std::bind (&ServerMethods::keepAlive,
                     this, std::placeholders::_1,
                     std::placeholders::_2) );
  handler.addMethod ("describe", std::bind (&ServerMethods::describe,
                     this, std::placeholders::_1,
                     std::placeholders::_2) );
}

ServerMethods::~ServerMethods()
{
}

static void
requireParams (const Json::Value &params)
{
  if (params == Json::Value::null) {
    throw JsonRpc::CallException (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                  "'params' is requiered");
  }
}

static bool
getOrCreateSessionId (std::string &_sessionId, const Json::Value &params)
{
  try {
    JsonRpc::getValue (params, SESSION_ID, _sessionId);
    return true;
  } catch (JsonRpc::CallException e) {
    _sessionId = generateUUID ();
    return false;
  }
}

void
ServerMethods::process (const std::string &request, std::string &response)
{
  handler.process (request, response);
}

void
ServerMethods::keepAliveSession (const std::string &sessionId)
{
  MediaSet::getMediaSet()->keepAliveSession (sessionId);
}

bool
ServerMethods::preProcess (const Json::Value &request, Json::Value &response)
{
  std::string sessionId;
  std::string resp;
  int requestId;

  try {
    Json::Reader reader;
    Json::Value params;

    JsonRpc::getValue (request, JSON_RPC_ID, requestId);
    JsonRpc::getValue (request, JSON_RPC_PARAMS, params);
    JsonRpc::getValue (params, SESSION_ID, sessionId);

    resp = cache->getCachedResponse (sessionId, requestId);

    GST_DEBUG ("Cached response: %s", resp.c_str() );

    /* update response with the one we got from cache */
    reader.parse (resp, response);

    return false;
  } catch (...) {
    /* continue processing */
    return true;
  }
}

void
ServerMethods::postProcess (const Json::Value &request, Json::Value &response)
{
  Json::FastWriter writer;
  std::string sessionId;
  std::string resp;
  int requestId;

  try {
    Json::Reader reader;

    JsonRpc::getValue (request, JSON_RPC_ID, requestId);

    try {
      Json::Value result;

      JsonRpc::getValue (response, JSON_RPC_RESULT, result);
      JsonRpc::getValue (result, SESSION_ID, sessionId);
    } catch (JsonRpc::CallException &ex) {
      Json::Value params;

      JsonRpc::getValue (request, JSON_RPC_PARAMS, params);
      JsonRpc::getValue (params, SESSION_ID, sessionId);
    }

    /* CacheException will be triggered if this response is not cached */
    cache->getCachedResponse (sessionId, requestId);
  } catch (JsonRpc::CallException &e) {
    /* We could not get some of the required parameters. Ignore */
  } catch (CacheException &e) {
    /* Cache response */
    resp = writer.write (response);

    GST_DEBUG ("Caching: %s", resp.c_str() );
    cache->addResponse (sessionId, requestId, resp);
  }
}

void
ServerMethods::describe (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObjectImpl> obj;
  std::string subscription;
  std::string sessionId;
  std::string objectId;

  requireParams (params);

  getOrCreateSessionId (sessionId, params);

  JsonRpc::getValue (params, OBJECT, objectId);

  try {
    obj = MediaSet::getMediaSet()->getMediaObject (sessionId, objectId);


  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }

  response[SESSION_ID] = sessionId;
  response["type"] = obj->getType ();
}

void
ServerMethods::keepAlive (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObject> obj;
  std::string subscription;
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, SESSION_ID, sessionId);

  try {
    MediaSet::getMediaSet()->keepAliveSession (sessionId);
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }
}

void
ServerMethods::release (const Json::Value &params, Json::Value &response)
{
  std::string objectId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);

  try {
    MediaSet::getMediaSet()->release (objectId);
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }
}

void
ServerMethods::ref (const Json::Value &params, Json::Value &response)
{
  std::string objectId;
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);
  JsonRpc::getValue (params, SESSION_ID, sessionId);

  try {
    MediaSet::getMediaSet()->ref (sessionId, objectId);
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }
}

void
ServerMethods::unref (const Json::Value &params, Json::Value &response)
{
  std::string objectId;
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);
  JsonRpc::getValue (params, SESSION_ID, sessionId);

  try {
    MediaSet::getMediaSet()->unref (sessionId, objectId);
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }
}

void
ServerMethods::unsubscribe (const Json::Value &params, Json::Value &response)
{
  std::string subscription;
  std::string sessionId;
  std::string objectId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);
  JsonRpc::getValue (params, SUBSCRIPTION, subscription);
  JsonRpc::getValue (params, SESSION_ID, sessionId);

  MediaSet::getMediaSet()->removeEventHandler (sessionId, objectId, subscription);
}

void
ServerMethods::registerEventHandler (std::shared_ptr<MediaObjectImpl> obj,
                                     const std::string &sessionId,
                                     const  std::string &subscriptionId,
                                     std::shared_ptr<EventHandler> handler)
{
  MediaSet::getMediaSet()->addEventHandler (sessionId, obj->getId(),
      subscriptionId, handler);
}


std::string
ServerMethods::connectEventHandler (std::shared_ptr<MediaObjectImpl> obj,
                                    const std::string &sessionId, const std::string &eventType,
                                    std::shared_ptr<EventHandler> handler)
{
  std::string subscriptionId;

  if (!obj->connect (eventType, handler) ) {
    throw KurentoException (MEDIA_OBJECT_EVENT_NOT_SUPPORTED, "Event not found");
  }

  subscriptionId = generateUUID();

  registerEventHandler (obj, sessionId, subscriptionId, handler);

  return subscriptionId;
}

void
ServerMethods::subscribe (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObjectImpl> obj;
  std::string eventType;
  std::string handlerId;
  std::string sessionId;
  std::string objectId;

  requireParams (params);

  JsonRpc::getValue (params, "type", eventType);
  JsonRpc::getValue (params, OBJECT, objectId);

  getOrCreateSessionId (sessionId, params);

  try {
    obj = MediaSet::getMediaSet()->getMediaObject (sessionId, objectId);

    try {
      handlerId = eventSubscriptionHandler (obj, sessionId, eventType, params);
    } catch (std::bad_function_call &e) {
      throw KurentoException (NOT_IMPLEMENTED,
                              "Current transport does not support events");
    }

    if (handlerId == "") {
      throw KurentoException (MEDIA_OBJECT_EVENT_NOT_SUPPORTED, "Event not found");
    }
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }

  response[SESSION_ID] = sessionId;
  response["value"] = handlerId;
}

void
ServerMethods::invoke (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObjectImpl> obj;
  std::string sessionId;
  std::string operation;
  Json::Value operationParams;
  std::string objectId;

  requireParams (params);

  JsonRpc::getValue (params, "operation", operation);
  JsonRpc::getValue (params, OBJECT, objectId);

  try {
    JsonRpc::getValue (params, "operationParams", operationParams);
  } catch (JsonRpc::CallException e) {
    /* operationParams is optional at this point */
  }

  getOrCreateSessionId (sessionId, params);

  try {
    Json::Value value;

    obj = MediaSet::getMediaSet()->getMediaObject (sessionId, objectId);

    if (!obj) {
      throw KurentoException (MEDIA_OBJECT_NOT_FOUND, "Object not found");
    }

    obj->invoke (obj, operation, operationParams, value);

    response["value"] = value;
    response[SESSION_ID] = sessionId;
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }
}

void
ServerMethods::connect (const Json::Value &params, Json::Value &response)
{
  std::string sessionId;

  if (params == Json::Value::null) {
    sessionId = generateUUID ();
  } else {
    bool doKeepAlive = false;

    doKeepAlive = getOrCreateSessionId (sessionId, params);

    if (doKeepAlive) {
      try {
        keepAliveSession (sessionId);
      } catch (KurentoException &ex) {
        Json::Value data;
        data["code"] = ex.getCode();
        data["message"] = ex.getMessage();

        JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                  ex.what(), data);
        throw e;
      }
    }
  }

  response[SESSION_ID] = sessionId;
  response["serverId"] = instanceId;
}

void
ServerMethods::create (const Json::Value &params,
                       Json::Value &response)
{
  std::string type;
  std::shared_ptr<Factory> factory;
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, "type", type);

  try {
    JsonRpc::getValue (params, SESSION_ID, sessionId);
  } catch (JsonRpc::CallException e) {
    sessionId = generateUUID ();
  }

  factory = moduleManager.getFactory (type);

  if (factory) {
    try {
      std::shared_ptr <MediaObjectImpl> object;

      object = std::dynamic_pointer_cast<MediaObjectImpl> (
                 factory->createObject (config, sessionId, params["constructorParams"]) );

      response["value"] = object->getId();
      response[SESSION_ID] = sessionId;
    } catch (KurentoException &ex) {
      Json::Value data;
      data["code"] = ex.getCode();
      data["message"] = ex.getMessage();

      JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                ex.what(), data);
      throw e;
    }
  } else {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Class '" + type + "' does not exist");
    // TODO: Define error data and code
    throw e;
  }

}

ServerMethods::StaticConstructor ServerMethods::staticConstructor;

ServerMethods::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}



} /* kurento */
