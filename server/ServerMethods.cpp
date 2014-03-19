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

#define GST_CAT_DEFAULT kurento_server_methods
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoServerMethods"

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
  handler.addMethod ("unref", std::bind (&ServerMethods::unref,
                                         this, std::placeholders::_1,
                                         std::placeholders::_2) );
  handler.addMethod ("keepAlive", std::bind (&ServerMethods::keepAlive,
                     this, std::placeholders::_1,
                     std::placeholders::_2) );
}

void
ServerMethods::process (const std::string &request, std::string &response)
{
  handler.process (request, response);
}

void
ServerMethods::keepAlive (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObject> obj;
  std::string subscription;

  if (params == Json::Value::null) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'params' is requiered");
    throw e;
  }

  if (!params["sessionId"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'sessionId' parameter should be a string");
    throw e;
  }

  try {
    MediaSet::getMediaSet()->keepAliveSession (params["sessionId"].asString () );
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  } catch (JsonRpc::CallException ) {
    throw;
  } catch (std::string &ex) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected error: " + ex);
    throw e;
  } catch (std::exception &ex) {
    std::string message = "Unexpected exception: ";

    message.append (ex.what() );
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT, message);
    throw e;
  } catch (...) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected exception");
    throw e;
  }
}

void
ServerMethods::release (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObject> obj;
  std::string subscription;

  if (params == Json::Value::null) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'params' is requiered");
    throw e;
  }

  if (!params["object"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'object' parameter should be a string");
    throw e;
  }

  try {
    MediaSet::getMediaSet()->release (params["object"].asString () );
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  } catch (JsonRpc::CallException ) {
    throw;
  } catch (std::string &ex) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected error: " + ex);
    throw e;
  } catch (std::exception &ex) {
    std::string message = "Unexpected exception: ";

    message.append (ex.what() );
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT, message);
    throw e;
  } catch (...) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected exception");
    throw e;
  }
}

void
ServerMethods::unref (const Json::Value &params, Json::Value &response)
{
  std::shared_ptr<MediaObject> obj;
  std::string subscription;

  if (params == Json::Value::null) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'params' is requiered");
    throw e;
  }

  if (!params["object"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'object' parameter should be a string");
    throw e;
  }

  if (!params["sessionId"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'sessionId' parameter should be a string");
    throw e;
  }

  try {
    MediaSet::getMediaSet()->unref (params["sessionId"].asString (),
                                    params["object"].asString () );
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  } catch (JsonRpc::CallException ) {
    throw;
  } catch (std::string &ex) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected error: " + ex);
    throw e;
  } catch (std::exception &ex) {
    std::string message = "Unexpected exception: ";

    message.append (ex.what() );
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT, message);
    throw e;
  } catch (...) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected exception");
    throw e;
  }
}

void
ServerMethods::unsubscribe (const Json::Value &params, Json::Value &response)
{
  std::string subscription;

  if (params == Json::Value::null) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'params' is requiered");
    throw e;
  }

  if (!params.isMember ("subscription") ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'subscription' parameter is required");
    throw e;
  }

  if (!params["subscription"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'subscription' parameter should be a string");
    throw e;
  }

  subscription = params["subscription"].asString();

  eventHandlers.erase (subscription);
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

  if (params == Json::Value::null) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'params' is requiered");
    throw e;
  }

  if (!params["type"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'type' parameter should be a string");
    throw e;
  }

  eventType = params["type"].asString();

  if (!params["ip"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'ip' parameter should be a string");
    throw e;
  }

  ip = params["ip"].asString();

  if (!params["port"].isInt() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'port' parameter should be a int");
    throw e;
  }

  port = params["port"].asInt();

  if (!params["object"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'object' parameter should be a string");
    throw e;
  }

  if (params.isMember ("sessionId") && params["sessionId"].isString() ) {
    sessionId = params["sessionId"].asString();
  } else {
    generateUUID (sessionId);
  }

  try {
    std::shared_ptr<EventHandler> handler (new EventHandler (ip, port) );

    obj = MediaObject::Factory::getObject (params["object"].asString () );
    MediaSet::getMediaSet()->ref (sessionId,
                                  std::dynamic_pointer_cast<MediaObjectImpl> (obj) );
    handlerId = obj->connect (eventType, handler);

    if (handlerId == "") {
      KurentoException e ("event not found");
      throw e;
    }

    eventHandlers[handlerId] = handler;
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  } catch (JsonRpc::CallException ) {
    throw;
  } catch (std::string &ex) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected error: " + ex);
    throw e;
  } catch (std::exception &ex) {
    std::string message = "Unexpected exception: ";

    message.append (ex.what() );
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT, message);
    throw e;
  } catch (...) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected exception");
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

  if (params == Json::Value::null) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'params' is requiered");
    throw e;
  }

  if (!params.isMember ("operation") ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'operation' parameter is requiered");
    throw e;
  }

  if (!params["operation"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'operation' parameter should be a string");
    throw e;
  }

  if (params.isMember ("operationParams") ) {
    if (!params["operationParams"].isObject() ) {
      JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                "'operation' parameter should be a string");
      throw e;
    }
  }

  if (!params.isMember ("object") ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'object' parameter is requiered");
    throw e;
  }

  if (!params["object"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'object' parameter should be a string");
    throw e;
  }

  if (params.isMember ("sessionId") && params["sessionId"].isString() ) {
    sessionId = params["sessionId"].asString();
  } else {
    generateUUID (sessionId);
  }

  obj = MediaPipeline::Factory::getObject (params["object"].asString () );
  MediaSet::getMediaSet()->ref (sessionId,
                                std::dynamic_pointer_cast<MediaObjectImpl> (obj) );

  if (!obj) {
    KurentoException e ("object not found");
    throw e;
  }

  try {
    Json::Value value;

    obj->getInvoker().invoke (obj, params["operation"].asString(),
                              params["operationParams"], value);

    response["value"] = value;
    response["sessionId"] = sessionId;
  } catch (KurentoException &ex) {
    Json::Value data;
    data["code"] = ex.getCode();
    data["message"] = ex.getMessage();

    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              ex.what(), data);
    throw e;
  } catch (JsonRpc::CallException ) {
    throw;
  } catch (std::string &ex) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected error: " + ex);
    throw e;
  } catch (std::exception &ex) {
    std::string message = "Unexpected exception: ";

    message.append (ex.what() );
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT, message);
    throw e;
  } catch (...) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "Unexpected exception");
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

  if (params == Json::Value::null) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'params' is requiered");
    throw e;
  }

  if (!params.isMember ("type") ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'type' parameter is requiered");
    throw e;
  }

  if (!params["type"].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'type' parameter should be a string");
    throw e;
  }

  if (params.isMember ("sessionId") && params["sessionId"].isString() ) {
    sessionId = params["sessionId"].asString();
  } else {
    generateUUID (sessionId);
  }

  type = params["type"].asString();

  if (!objectRegistrar) {
    KurentoException e ("Class '" + type + "' does not exist");
    throw e;
  }

  factory = objectRegistrar->getFactory (type);

  if (factory) {
    try {
      std::shared_ptr <MediaObjectImpl> object;

      object = std::dynamic_pointer_cast<MediaObjectImpl> (
                 factory->createObject (params["constructorParams"]) );
      MediaSet::getMediaSet()->ref (sessionId, object);

      try {
        object->getMediaPipeline();
      } catch (...) {
        GST_ERROR ("Error getting pipeline");
      }

      MediaSet::getMediaSet()->ref (sessionId, object);
      response["value"] = object->getIdStr();
      response["sessionId"] = sessionId;
    } catch (KurentoException &ex) {
      Json::Value data;
      data["code"] = ex.getCode();
      data["message"] = ex.getMessage();

      JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                ex.what(), data);
      throw e;
    } catch (JsonRpc::CallException ) {
      throw;
    } catch (std::string &ex) {
      JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                "Unexpected error: " + ex);
      throw e;
    } catch (std::exception &ex) {
      std::string message = "Unexpected exception: ";

      message.append (ex.what() );
      JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT, message);
      throw e;
    } catch (...) {
      JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                                "Unexpected exception");
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
