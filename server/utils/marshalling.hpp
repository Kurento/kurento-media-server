/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
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

#ifndef __MARSHALLING_HPP__
#define __MARSHALLING_HPP__

#include "KmsMediaServer_types.h"
#include "KmsMediaHandler_types.h"

namespace kurento
{

std::string marshalString (const std::string &str) throw (KmsMediaServerException);
std::string unmarshalString (const std::string &data) throw (KmsMediaServerException);

std::shared_ptr<KmsMediaParams> createStringParams (const std::string &data) throw (KmsMediaServerException);
std::string unmarshalStringParams (const KmsMediaParams &params) throw (KmsMediaServerException);

inline std::shared_ptr<KmsMediaCommandResult> createStirngCommandResult (const std::string &data)
throw (KmsMediaServerException)
{
  return (std::shared_ptr<KmsMediaCommandResult>) createStringParams (data);
}

inline std::string unmarshalStringCommandResult (const KmsMediaCommandResult &commandResult)
throw (KmsMediaServerException)
{
  return unmarshalStringParams ((KmsMediaParams) commandResult);
}

inline std::shared_ptr<KmsMediaEventData> createStringEventData (const std::string &data) throw (KmsMediaServerException)
{
  return (std::shared_ptr<KmsMediaEventData>) createStringParams (data);
}

inline std::string unmarshalStringEventData (const KmsMediaEventData &eventData)
throw (KmsMediaServerException)
{
  return unmarshalStringParams ((KmsMediaEventData) eventData);
}

std::shared_ptr<KmsMediaCommand> createVoidCommand (std::string commandName);
std::shared_ptr<KmsMediaCommand> createStringCommand (std::string commandName, std::string str) throw (KmsMediaServerException);

} // kurento

#endif /* __MARSHALLING_HPP__ */
