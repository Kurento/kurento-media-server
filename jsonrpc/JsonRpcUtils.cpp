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

#include "JsonRpcUtils.hpp"
#include "JsonRpcException.hpp"

namespace kurento
{
namespace JsonRpc
{

static void
checkParameter (const Json::Value &params, const std::string &name)
{
  if (!params.isMember (name) ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'" + name + "' parameter is required");
    throw e;
  }
}

void
getValue (const Json::Value &params, const std::string &name,
          std::string &_return)
{
  checkParameter (params, name);

  if (!params[name].isString() ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'" + name + "' parameter should be a string");
    throw e;
  }

  _return = params[name].asString();
}

void
getValue (const Json::Value &params, const std::string &name,
          int &_return)
{
  checkParameter (params, name);

  if (!params[name].isInt () ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'" + name + "' parameter should be an integer");
    throw e;
  }

  _return = params[name].asInt ();
}

void
getValue (const Json::Value &params, const std::string &name,
          Json::Value &_return)
{
  checkParameter (params, name);

  if (!params[name].isObject () ) {
    JsonRpc::CallException e (JsonRpc::ErrorCode::SERVER_ERROR_INIT,
                              "'" + name + "' parameter should be an object");
    throw e;
  }

  _return = params[name];
}

} /* JsonRpc */
} /* kurento */
