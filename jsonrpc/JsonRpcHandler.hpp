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

#ifndef __JSONRPC_HANDLER_HPP__
#define __JSONRPC_HANDLER_HPP__

#include <string>
#include <functional>
#include <memory>
#include <map>

#include <json/json.h>
#include "JsonRpcException.hpp"

namespace kurento
{
namespace JsonRpc
{

class Handler
{
public:

  Handler() {};
  virtual ~Handler() {};

  typedef std::function<void (const Json::Value &, Json::Value &) >
  Method;

  void addMethod (const std::string &name, Method method);
  bool process (const std::string &msg, std::string &_responseMsg);

private:

  std::map<std::string, Method> methods;
  bool process (const Json::Value &msg, Json::Value &_response);
  bool checkProtocol (const Json::Value &root, Json::Value &error);
};

} /* JsonRpc */
} /* kurento  */

#endif /* __JSONRPC_HANDLER_HPP__ */
