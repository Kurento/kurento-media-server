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

#include <gst/gst.h>
#include "ServerMethods.hpp"
#include <MediaSet.hpp>
#include <memory>
#include <string>
#include <EventHandler.hpp>
#include <KurentoException.hpp>
#include <jsonrpc/JsonRpcException.hpp>
#include <jsonrpc/JsonRpcUtils.hpp>
#include <jsonrpc/JsonRpcConstants.hpp>
#include <jsonrpc/JsonFixes.hpp>

#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "modules.hpp"
#include <version.hpp>
#include <ServerManagerImpl.hpp>
#include <ServerInfo.hpp>
#include <ModuleInfo.hpp>
#include <ServerType.hpp>
#include <UUIDGenerator.hpp>

#include <ResourceManager.hpp>

#define GST_CAT_DEFAULT kurento_server_methods
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoServerMethods"

#define SESSION_ID "sessionId"
#define VALUE "value"
#define OBJECT "object"
#define SUBSCRIPTION "subscription"
#define TYPE "type"
#define QUALIFIED_TYPE "qualifiedType"
#define HIERARCHY "hierarchy"

#define REQUEST_TIMEOUT 20000 /* 20 seconds */

static const std::string KURENTO_MODULES_PATH = "KURENTO_MODULES_PATH";
static const std::string NEW_REF = "newref:";

namespace kurento
{

ServerMethods::ServerMethods (const boost::property_tree::ptree &config) :
  config (config), moduleManager (getModuleManager() )
{
  std::string version (get_version() );
  std::vector<std::shared_ptr<ModuleInfo>> modules;
  std::shared_ptr<ServerType> type (new ServerType (ServerType::KMS) );
  std::vector<std::string> capabilities;
  std::shared_ptr <ServerInfo> serverInfo;
  std::shared_ptr<MediaObjectImpl> serverManager;
  std::chrono::seconds collectorInterval{};
  bool disableRequestCache;

  collectorInterval = std::chrono::seconds (
                        config.get<long> ("mediaServer.resources.garbageCollectorPeriod",
                                         MediaSet::getCollectorInterval().count() ) );
  MediaSet::setCollectorInterval (collectorInterval);

  disableRequestCache = config.get<bool> ("mediaServer.resources.disableRequestCache",
                                          false);

  resourceLimitPercent =
    config.get<float> ("mediaServer.resources.exceptionLimit",
                       DEFAULT_RESOURCE_LIMIT_PERCENT);

  GST_INFO ("Using above %.0f%% of system limits will throw NOT_ENOUGH_RESOURCES exception",
            resourceLimitPercent * 100.0f);

  std::string maxThreadsStr;
  const rlim_t maxThreads = getMaxThreads ();
  if (maxThreads == RLIM_INFINITY) {
    maxThreadsStr = "unlimited";
  } else {
    maxThreadsStr = std::to_string(maxThreads);
  }

  std::string maxOpenFilesStr;
  const rlim_t maxOpenFiles = getMaxOpenFiles ();
  if (maxOpenFiles == RLIM_INFINITY) {
    maxOpenFilesStr = "unlimited";
  } else {
    maxOpenFilesStr = std::to_string(maxOpenFiles);
  }

  GST_INFO ("System limits: %s threads, %s files",
      maxThreadsStr.c_str(), maxOpenFilesStr.c_str());

  instanceId = generateUUID();

  for (auto moduleIt : moduleManager.getModules () ) {
    std::vector<std::string> factories;

    for (auto factIt : moduleIt.second->getFactories() ) {
      factories.push_back (factIt.first);
    }

    modules.push_back(std::make_shared<ModuleInfo>(
        moduleIt.second->getVersion(), moduleIt.second->getName(),
        moduleIt.second->getGenerationTime(), factories));
  }

  capabilities.emplace_back("transactions");

  serverInfo =
      std::make_shared<ServerInfo>(version, modules, type, capabilities);

  serverManager = MediaSet::getMediaSet ()->ref (new ServerManagerImpl (
                    serverInfo, config, moduleManager) );
  MediaSet::getMediaSet ()->setServerManager (std::dynamic_pointer_cast
      <ServerManagerImpl> (serverManager) );

  cache = std::make_shared<RequestCache>(REQUEST_TIMEOUT);

  if (!disableRequestCache) {
    handler.setPreProcess (std::bind (&ServerMethods::preProcess, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2) );
    handler.setPostProcess (std::bind (&ServerMethods::postProcess, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2) );
    GST_INFO ("RPC Request Cache is ENABLED");
  } else {
    GST_INFO ("RPC Request Cache is DISABLED");
  }

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
  handler.addMethod ("transaction", std::bind (&ServerMethods::transaction,
                     this, std::placeholders::_1, std::placeholders::_2) );
  handler.addMethod ("ping", std::bind (&ServerMethods::ping, this,
                                        std::placeholders::_1, std::placeholders::_2) );
  handler.addMethod ("closeSession", std::bind (&ServerMethods::closeSession,
                     this,
                     std::placeholders::_1, std::placeholders::_2) );
}

ServerMethods::~ServerMethods() = default;

static void
requireParams (const Json::Value &params)
{
  if (params == Json::Value::null) {
    Json::Value data;

    data[TYPE] = "INVALID_PARAMS";

    throw JsonRpc::CallException (JsonRpc::ErrorCode::INVALID_PARAMS,
                                  "'params' is requiered", data);
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

static std::string
getSessionId (const Json::Value &resp)
{
  std::string sessionId;

  Json::Value result;

  if (resp.isMember (JSON_RPC_ERROR) ) {
    /* If response is an error do not return a sessionId */
    return sessionId;
  }

  JsonRpc::getValue (resp, JSON_RPC_RESULT, result);
  JsonRpc::getValue (result, SESSION_ID, sessionId);

  return sessionId;
}


static void
injectSessionId (Json::Value &req, const std::string &sessionId)
{
  try {
    Json::Value params;

    params = req[JSON_RPC_PARAMS];

    try {
      std::string oldSessionId;

      JsonRpc::getValue (params, SESSION_ID, oldSessionId);
    } catch (JsonRpc::CallException &e) {
      // There is no sessionId, inject it
      GST_TRACE ("Injecting sessionId %s", sessionId.c_str() );
      params[SESSION_ID] = sessionId;
      req[JSON_RPC_PARAMS] = params;
    }
  } catch (JsonRpc::CallException &ex) {

  }
}

std::string
ServerMethods::process (const std::string &requestStr, std::string &responseStr,
                        std::string &sessionId)
{
  Json::Value response;
  Json::Value request;
  bool parse = false;
  Json::Reader reader;
  std::string newSessionId;

  parse = reader.parse (requestStr, request);

  if (!parse) {
    throw JsonRpc::CallException (JsonRpc::ErrorCode::PARSE_ERROR, "Parse error.");
  }

  if (!sessionId.empty() ) {
    injectSessionId (request, sessionId);
  }

  handler.process (request, response);

  try {
    newSessionId = getSessionId (response);
  } catch (JsonRpc::CallException &ex) {
    /* We could not get some of the required parameters. Ignore */
    newSessionId = sessionId;
  }

  if (response != Json::Value::null) {
    Json::StreamWriterBuilder writerFactory;
    writerFactory["indentation"] = "";
    responseStr = Json::writeString (writerFactory, response);
  }

  return newSessionId;
}

void
ServerMethods::keepAliveSession (const std::string &sessionId)
{
  MediaSet::getMediaSet()->keepAliveSession (sessionId);
}

bool
ServerMethods::preProcess (const Json::Value &request, Json::Value &response)
{
  std::string sessionId;//   std::string resp;
  std::string requestId;

  try {
    Json::Value params;

    JsonRpc::getValue (request, JSON_RPC_ID, requestId);
    JsonRpc::getValue (request, JSON_RPC_PARAMS, params);
    JsonRpc::getValue (params, SESSION_ID, sessionId);

    response = cache->getCachedResponse (sessionId, requestId);

    GST_DEBUG ("Cached response");

    return false;
  } catch (...) {
    /* continue processing */
    return true;
  }
}

void
ServerMethods::postProcess (const Json::Value &request, Json::Value &response)
{
  std::string sessionId;
  std::string requestId;

  try {
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
    GST_LOG ("Caching: %s", response.toStyledString().c_str() );
    cache->addResponse (sessionId, requestId, response);
  }
}

void
ServerMethods::describe (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObjectImpl> obj;
  std::string sessionId;
  std::string objectId;

  requireParams (params);

  getOrCreateSessionId (sessionId, params);

  JsonRpc::getValue (params, OBJECT, objectId);

  try {
    obj = MediaSet::getMediaSet()->getMediaObject (sessionId, objectId);

  } catch (KurentoException &ex) {
    Json::Value data;

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
  }

  response[SESSION_ID] = sessionId;
  response[TYPE] = obj->getType ();
  response[QUALIFIED_TYPE] = obj->getQualifiedType();
  JsonSerializer serializer (true);
  std::vector <std::string> array = obj->getHierarchy();
  serializer.Serialize ("array", array);
  response[HIERARCHY]  = serializer.JsonValue["array"];
}

void
ServerMethods::keepAlive (const Json::Value &params, Json::Value &response)
{
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, SESSION_ID, sessionId);

  try {
    MediaSet::getMediaSet()->keepAliveSession (sessionId);
  } catch (KurentoException &ex) {
    Json::Value data;

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
  }

  response[SESSION_ID] = sessionId;
}

void
ServerMethods::release (const Json::Value &params, Json::Value &response)
{
  std::string objectId;
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);

  getOrCreateSessionId (sessionId, params);

  try {
    MediaSet::getMediaSet()->release (objectId);
  } catch (KurentoException &ex) {
    Json::Value data;

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
  }

  response[SESSION_ID] = sessionId;
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

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
  }

  response[SESSION_ID] = sessionId;
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

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
  }

  response[SESSION_ID] = sessionId;
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

  response[SESSION_ID] = sessionId;
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

  JsonRpc::getValue (params, TYPE, eventType);
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

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
  }

  response[SESSION_ID] = sessionId;
  response[VALUE] = handlerId;
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

    response[VALUE] = value;
    response[SESSION_ID] = sessionId;
  } catch (KurentoException &ex) {
    Json::Value data;

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
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

        data[TYPE] = ex.getType();

        throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
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

  JsonRpc::getValue (params, TYPE, type);

  try {
    JsonRpc::getValue (params, SESSION_ID, sessionId);
  } catch (JsonRpc::CallException e) {
    sessionId = generateUUID ();
  }

  try {
    factory = moduleManager.getFactory (type);

    checkResources (resourceLimitPercent);

    std::shared_ptr <MediaObjectImpl> object;

    object = std::dynamic_pointer_cast<MediaObjectImpl> (
               factory->createObject (config, sessionId, params["constructorParams"]) );

    response[VALUE] = object->getId();
    response[SESSION_ID] = sessionId;
  } catch (KurentoException &ex) {
    Json::Value data;

    data[TYPE] = ex.getType();

    throw JsonRpc::CallException (ex.getCode (), ex.getMessage (), data);
  }
}

void
insertResult (Json::Value &value, Json::Value &responses, const int index)
{
  try {
    Json::Value result;

    JsonRpc::getValue (responses[index], JSON_RPC_RESULT, result);
    value = result[VALUE];
  } catch (JsonRpc::CallException e) {
    Json::Value data;

    KurentoException ke (MALFORMED_TRANSACTION,
                         "Result not found on request " +
                         std::to_string (index) );

    data[TYPE] = ke.getType();

    GST_ERROR ("Error while inserting new ref value: %s",
               e.getMessage ().c_str () );
    throw JsonRpc::CallException (ke.getCode (), ke.getMessage (), data);
  }
}

void
injectRefs (Json::Value &params, Json::Value &responses)
{
  if (params.isObject () || params.isArray () ) {
    for (auto &param : params) {
      injectRefs(param, responses);
    }
  } else if (params.isConvertibleTo (Json::ValueType::stringValue) ) {
    std::string param = JsonFixes::getString (params);

    if (param.size() > NEW_REF.size()
        && param.substr (0, NEW_REF.size() ) == NEW_REF) {
      std::string ref = param.substr (NEW_REF.size() );

      try {
        int index = stoi (ref);

        insertResult (params, responses, index);
      } catch (std::invalid_argument &e) {
        Json::Value data;

        KurentoException ke (MALFORMED_TRANSACTION,
                             "Invalid index of newref '" + ref + "'");

        data[TYPE] = ke.getType();

        throw JsonRpc::CallException (ke.getCode (), ke.getMessage (), data);
      }
    }
  }
}

void
ServerMethods::transaction (const Json::Value &params, Json::Value &response)
{
  std::string sessionId;
  Json::Value operations;
  std::string uniqueId = generateUUID();

  requireParams (params);

  getOrCreateSessionId (sessionId, params);

  JsonRpc::getArray (params, "operations", operations);
  Json::Value responses;

  for (uint i = 0; i < operations.size(); i++) {
    bool ret;

    Json::Value &reqParams = operations[i][JSON_RPC_PARAMS];

    reqParams[SESSION_ID] = sessionId;

    if (!operations[i][JSON_RPC_ID].isConvertibleTo (Json::ValueType::uintValue)
        || operations[i][JSON_RPC_ID].asUInt() != i) {
      Json::Value data;

      KurentoException ke (MALFORMED_TRANSACTION,
                           "Id of request '" + std::to_string (i) +
                           "' should be '" + std::to_string (i) + "'");

      data[TYPE] = ke.getType();

      throw JsonRpc::CallException (ke.getCode (), ke.getMessage (), data);
    }

    try {
      operations[i][JSON_RPC_ID] = uniqueId + "_" + std::to_string (
                                     operations[i][JSON_RPC_ID].asUInt() );
    } catch (...) {
      GST_ERROR ("Error setting id");
    }

    injectRefs (reqParams, responses);

    ret = handler.process (operations[i], responses[i]);

    responses[i][JSON_RPC_ID] = i;

    if (!ret) {
      break;
    }
  }

  if (responses.isNull () ) {
    responses.resize (0);
  }

  response[VALUE] = responses;
  response[SESSION_ID] = sessionId;
}

void
ServerMethods::ping (const Json::Value &params, Json::Value &response)
{
  std::string sessionId;

  try {
    JsonRpc::getValue (params, SESSION_ID, sessionId);
    response [SESSION_ID] = sessionId;
  } catch (JsonRpc::CallException e) {
  }

  if (sessionId.empty()) {
    GST_INFO ("WebSocket Ping/Pong");
  }
  else {
    GST_INFO ("WebSocket Ping/Pong with sessionId %s", sessionId.c_str());
  }

  response[VALUE] = "pong";
}

void
ServerMethods::closeSession (const Json::Value &params,
                             Json::Value &response)
{
  std::string sessionId;
  bool release = false;

  requireParams (params);

  try {
    JsonRpc::getValue (params, "release", release);
  } catch (JsonRpc::CallException e) {
    /* release param is optional*/
  }

  JsonRpc::getValue (params, SESSION_ID, sessionId);

  if (release) {
    MediaSet::getMediaSet()->releaseSession (sessionId);
  } else {
    MediaSet::getMediaSet()->unrefSession (sessionId);
  }
}

ServerMethods::StaticConstructor ServerMethods::staticConstructor;

ServerMethods::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
