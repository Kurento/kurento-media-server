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

#include "JsonRpcHandler.hpp"
#include "JsonRpcConstants.hpp"

#define JSON_RPC_ERROR_INVALID_REQUEST "Invalid JSON-RPC request."

namespace kurento
{
namespace JsonRpc
{

void
Handler::addMethod (const std::string &name, Method method)
{
  methods[name] = method;
}

bool
Handler::checkProtocol (const Json::Value &msg, Json::Value &error)
{
  Json::Value err;

  if (!msg.isObject() || !msg.isMember (JSON_RPC_PROTO) ||
      msg[JSON_RPC_PROTO] != JSON_RPC_PROTO_VERSION) {
    error[JSON_RPC_ID] = Json::Value::null;
    error[JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;

    err[JSON_RPC_ERROR_CODE] = INVALID_REQUEST;
    err[JSON_RPC_ERROR_MESSAGE] = JSON_RPC_ERROR_INVALID_REQUEST;
    error[JSON_RPC_ERROR] = err;
    return false;
  }

  if (msg.isMember (JSON_RPC_ID) && (msg[JSON_RPC_ID].isArray()
                                     || msg[JSON_RPC_ID].isObject() ) ) {
    error[JSON_RPC_ID] = Json::Value::null;
    error[JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;

    err[JSON_RPC_ERROR_CODE] = INVALID_REQUEST;
    err[JSON_RPC_ERROR_MESSAGE] = JSON_RPC_ERROR_INVALID_REQUEST;
    error[JSON_RPC_ERROR] = err;
    return false;
  }

  if (!msg.isMember (JSON_RPC_METHOD) || !msg[JSON_RPC_METHOD].isString() ) {
    error[JSON_RPC_ID] = Json::Value::null;
    error[JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;

    err[JSON_RPC_ERROR_CODE] = INVALID_REQUEST;
    err[JSON_RPC_ERROR_MESSAGE] = JSON_RPC_ERROR_INVALID_REQUEST;
    error[JSON_RPC_ERROR] = err;
    return false;
  }

  return true;
}

bool
Handler::process (const Json::Value &msg, Json::Value &_response)
{
  Json::Value error;
  std::string methodName;

  if (!checkProtocol (msg, error) ) {
    _response = error;
    return false;
  }

  _response[JSON_RPC_ID] = msg.isMember (JSON_RPC_ID) ? msg[JSON_RPC_ID] :
                           Json::Value::null;
  _response[JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;

  methodName = msg[JSON_RPC_METHOD].asString();

  if (methodName != "" && methods.find (methodName) != methods.end() ) {
    Method &method = methods[methodName];
    Json::Value response;

    try {
      method (msg[JSON_RPC_PARAMS], response);
      _response[JSON_RPC_RESULT] = response;
      return true;
    } catch (CallException &e) {
      Json::Value error;
      Json::Value data;

      error[JSON_RPC_ERROR_CODE] = e.getCode();
      error[JSON_RPC_ERROR_MESSAGE] = e.getMessage();

      data = e.getData();

      if (data != Json::Value::null) {
        error[JSON_RPC_ERROR_DATA] = data;
      }

      _response[JSON_RPC_ERROR] = error;
      return false;
    }
  }

  error[JSON_RPC_ERROR_CODE] = METHOD_NOT_FOUND;
  error[JSON_RPC_ERROR_MESSAGE] = "Method not found.";
  _response[JSON_RPC_ERROR] = error;

  return false;
}

bool
Handler::process (const std::string &msg, std::string &_responseMsg)
{
  Json::Value response;
  Json::Value request;
  Json::Value error;
  bool parse = false;
  Json::Reader reader;
  Json::FastWriter writer;

  parse = reader.parse (msg, request);

  if (!parse) {
    response[JSON_RPC_ID] = Json::Value::null;
    response[JSON_RPC_PROTO] = JSON_RPC_PROTO_VERSION;

    error[JSON_RPC_ERROR_CODE] = PARSE_ERROR;
    error[JSON_RPC_ERROR_MESSAGE] = "Parse error.";
    response[JSON_RPC_ERROR] = error;
    _responseMsg = writer.write (response);
    return false;
  }

  if (request.isArray() ) {
    Json::Value::ArrayIndex i = 0;
    Json::Value::ArrayIndex j = 0;

    for (i = 0 ; i < request.size() ; i++) {
      Json::Value ret;

      process (request[i], ret);

      if (ret != Json::Value::null) {
        response[j] = ret;
        j++;
      }
    }

    _responseMsg = writer.write (response);
    return true;
  } else {
    bool ret;

    ret = process (request, response);
    _responseMsg = writer.write (response);
    return ret;
  }
}

} /* JsonRpc */
} /* kurento */

