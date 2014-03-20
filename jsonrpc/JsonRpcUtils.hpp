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

#ifndef __JSONRPC_UTILS_HPP__
#define __JSONRPC_UTILS_HPP__

#include <json/json.h>

namespace kurento
{
namespace JsonRpc
{

void getValue (const Json::Value &params, const std::string &name,
               std::string &_return);

void getValue (const Json::Value &params, const std::string &name,
               int &_return);

void getValue (const Json::Value &params, const std::string &name,
               Json::Value &_return);

} /* JsonRpc */
} /* kurento */

#endif /* __JSONRPC_UTILS_HPP__ */
