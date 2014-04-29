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
#include <ServerMethods.hpp>
#include <types/MediaPipelineImpl.hpp>
#include <common/MediaSet.hpp>
#include <string>
#include <EventHandler.hpp>
#include <KurentoException.hpp>
#include "utils/utils.hpp"
#include <JsonRpcUtils.hpp>

#define GST_CAT_DEFAULT kurento_server_methods
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoServerMethods"

#define SESSION_ID "sessionId"
#define OBJECT "object"
#define SUBSCRIPTION "subscription"

namespace kurento
{

ServerMethods::ServerMethods()
{
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

static void
requireParams (const Json::Value &params)
{
  if (params == Json::Value::null) {
    throw JsonRpc::CallException (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                  "'params' is requiered");
  }
}

void
ServerMethods::process (const std::string &request, std::string &response)
{
  handler.process (request, response);
}

void
ServerMethods::describe (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObjectImpl> obj;
  std::string subscription;
  std::string sessionId;
  std::string objectId;

  requireParams (params);

  try {
    JsonRpc::getValue (params, SESSION_ID, sessionId);
  } catch (JsonRpc::CallException e) {
    generateUUID (sessionId);
  }

  JsonRpc::getValue (params, OBJECT, objectId);

  try {
    obj = MediaSet::getMediaSet().getMediaObject (sessionId, objectId);


  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }

  response["sessionId"] = sessionId;
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
    MediaSet::getMediaSet().keepAliveSession (sessionId);
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
  std::shared_ptr<MediaObject> obj;
  std::string subscription;
  std::string objectId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);

  try {
    MediaSet::getMediaSet().release (objectId);
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
  std::shared_ptr<MediaObject> obj;
  std::string objectId;
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);
  JsonRpc::getValue (params, OBJECT, sessionId);

  try {
    MediaSet::getMediaSet().ref (sessionId, objectId);
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
  std::shared_ptr<MediaObject> obj;
  std::string subscription;
  std::string objectId;
  std::string sessionId;

  requireParams (params);

  JsonRpc::getValue (params, OBJECT, objectId);
  JsonRpc::getValue (params, OBJECT, sessionId);

  try {
    MediaSet::getMediaSet().unref (sessionId, objectId);
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

  requireParams (params);

  JsonRpc::getValue (params, SUBSCRIPTION, subscription);
  JsonRpc::getValue (params, SESSION_ID, sessionId);

  MediaSet::getMediaSet().removeEventHandler (sessionId, subscription);
}

void
ServerMethods::subscribe (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObject> obj;
  std::string eventType;
  std::string ip;
  int port;
  std::string handlerId;
  std::string sessionId;
  std::string objectId;

  requireParams (params);

  JsonRpc::getValue (params, "type", eventType);
  JsonRpc::getValue (params, "ip", ip);
  JsonRpc::getValue (params, "port", port);
  JsonRpc::getValue (params, OBJECT, objectId);

  try {
    JsonRpc::getValue (params, SESSION_ID, sessionId);
  } catch (JsonRpc::CallException e) {
    generateUUID (sessionId);
  }

  try {
    std::shared_ptr<EventHandler> handler (new EventHandler (sessionId, objectId,
                                           ip, port) );

    obj = MediaSet::getMediaSet().getMediaObject (sessionId, objectId);
    handlerId = obj->connect (eventType, handler);

    if (handlerId == "") {
      KurentoException e (MEDIA_OBJECT_EVENT_NOT_SUPPORTED, "Event not found");
      throw e;
    }

    MediaSet::getMediaSet().addEventHandler (sessionId, handler);
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  }

  response["sessionId"] = sessionId;
  response["value"] = handlerId;
}

void
ServerMethods::invoke (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObject> obj;
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

  try {
    JsonRpc::getValue (params, SESSION_ID, sessionId);
  } catch (JsonRpc::CallException e) {
    generateUUID (sessionId);
  }

  try {
    Json::Value value;

    obj = MediaSet::getMediaSet().getMediaObject (sessionId, objectId);

    if (!obj) {
      throw KurentoException (MEDIA_OBJECT_NOT_FOUND, "Object not found");
    }

    obj->getInvoker().invoke (obj, operation, operationParams, value);

    response["value"] = value;
    response["sessionId"] = sessionId;
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
    generateUUID (sessionId);
  }

  if (!objectRegistrar) {
    KurentoException e (MEDIA_OBJECT_TYPE_NOT_FOUND,
                        "Class '" + type + "' does not exist");
    throw e;
  }

  factory = objectRegistrar->getFactory (type);

  if (factory) {
    try {
      std::shared_ptr <MediaObjectImpl> object;

      object = std::dynamic_pointer_cast<MediaObjectImpl> (
                 factory->createObject (params["constructorParams"]) );
      MediaSet::getMediaSet().ref (sessionId, object);

      try {
        object->getMediaPipeline();
      } catch (...) {
        GST_ERROR ("Error getting pipeline");
      }

      MediaSet::getMediaSet().ref (sessionId, object);
      response["value"] = object->getIdStr();
      response["sessionId"] = sessionId;
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
