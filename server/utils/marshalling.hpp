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
#include "KmsMediaServer_constants.h"
#include "KmsMediaHandler_types.h"

#include "KmsMediaObject_types.h"
#include "KmsMediaUriEndPointType_types.h"

namespace kurento
{

std::string marshalI32 (const int32_t i) throw (KmsMediaServerException);
int32_t unmarshalI32 (const std::string &data) throw (KmsMediaServerException);
std::string marshalString (const std::string &str) throw (KmsMediaServerException);
std::string unmarshalString (const std::string &data) throw (KmsMediaServerException);

std::shared_ptr<KmsMediaParam> createI32Param (const int32_t i) throw (KmsMediaServerException);
int32_t unmarshalI32Param (const KmsMediaParam &param) throw (KmsMediaServerException);
std::shared_ptr<KmsMediaParam> createStringParam (const std::string &data) throw (KmsMediaServerException);
std::string unmarshalStringParam (const KmsMediaParam &param) throw (KmsMediaServerException);

inline std::shared_ptr<KmsMediaInvocationReturn> createStringInvocationReturn (const std::string &data)
throw (KmsMediaServerException)
{
  return (std::shared_ptr<KmsMediaInvocationReturn>) createStringParam (data);
}

inline std::string unmarshalStringInvocationReturn (const KmsMediaInvocationReturn &invocationReturn)
throw (KmsMediaServerException)
{
  return unmarshalStringParam ( (KmsMediaParam) invocationReturn);
}

inline std::shared_ptr<KmsMediaEventData> createStringEventData (const std::string &data) throw (KmsMediaServerException)
{
  return (std::shared_ptr<KmsMediaEventData>) createStringParam (data);
}

inline std::string unmarshalStringEventData (const KmsMediaEventData &eventData)
throw (KmsMediaServerException)
{
  return unmarshalStringParam ( (KmsMediaEventData) eventData);
}

std::map<std::string, KmsMediaParam> createKmsMediaObjectConstructorParams (bool excludeFromGC,
    int32_t garbageCollectorPeriod = g_KmsMediaServer_constants.DEFAULT_GARBAGE_COLLECTOR_PERIOD)
throw (KmsMediaServerException);
std::string marshalKmsMediaObjectConstructorParams (KmsMediaObjectConstructorParams &moParams)
throw (KmsMediaServerException);
std::shared_ptr<KmsMediaObjectConstructorParams> unmarshalKmsMediaObjectConstructorParams (std::string data)
throw (KmsMediaServerException);

std::map<std::string, KmsMediaParam> createKmsMediaUriEndPointConstructorParams (std::string uri)
throw (KmsMediaServerException);
std::string marshalKmsMediaUriEndPointConstructorParams (KmsMediaUriEndPointConstructorParams &uriEpParams)
throw (KmsMediaServerException);
std::shared_ptr<KmsMediaUriEndPointConstructorParams> unmarshalKmsMediaUriEndPointConstructorParams (std::string data)
throw (KmsMediaServerException);

} // kurento

#endif /* __MARSHALLING_HPP__ */
