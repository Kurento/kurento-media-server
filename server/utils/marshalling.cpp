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

#include "marshalling.hpp"

#include "utils.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "KmsMediaUriEndPointType_constants.h"

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

std::string
marshalI32 (const int32_t i) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);
  std::string data;

  try {
    protocol.writeI32 (i);
    transport->appendBufferToString (data);
  } catch (...) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,
        "Cannot marshal I32");
  }

  return data;
}

int32_t
unmarshalI32 (const std::string &data) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport;
  int32_t i;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    protocol.readI32 (i);
  } catch (...) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
        "Cannot unmarshal I32");
  }

  return i;
}

std::string
marshalString (const std::string &str) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);
  std::string data;

  try {
    protocol.writeString (str);
    transport->appendBufferToString (data);
  } catch (...) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,
        "Cannot marshal string");
  }

  return data;
}

std::string
unmarshalString (const std::string &data) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport;
  std::string str;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    protocol.readString (str);
  } catch (...) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
        "Cannot unmarshal string");
  }

  return str;
}

std::shared_ptr<KmsMediaParam>
createI32Param (const int32_t i) throw (KmsMediaServerException)
{
  std::shared_ptr<KmsMediaParam> result (new KmsMediaParam () );

  result->__set_dataType (g_KmsMediaDataType_constants.I32_DATA_TYPE);
  result->__set_data (marshalI32 (i) );

  return result;
}

int32_t
unmarshalI32Param (const KmsMediaParam &param) throw (KmsMediaServerException)
{
  int32_t i;

  if (g_KmsMediaDataType_constants.I32_DATA_TYPE.compare (param.dataType) == 0) {
    i = unmarshalI32 (param.data);
  } else {
    throw createKmsMediaServerException  (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
        "Param is not of 'I32' data type");
  }

  return i;
}

std::shared_ptr<KmsMediaParam>
createStringParam (const std::string &data) throw (KmsMediaServerException)
{
  std::shared_ptr<KmsMediaParam> result (new KmsMediaParam () );

  result->__set_dataType (g_KmsMediaDataType_constants.STRING_DATA_TYPE);
  result->__set_data (marshalString (data) );

  return result;
}

std::string
unmarshalStringParam (const KmsMediaParam &param) throw (KmsMediaServerException)
{
  std::string str;

  if (g_KmsMediaDataType_constants.STRING_DATA_TYPE.compare (param.dataType) == 0) {
    str = unmarshalString (param.data);
  } else {
    throw createKmsMediaServerException  (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
        "Param is not of 'String' data type");
  }

  return str;
}

std::map<std::string, KmsMediaParam>
createKmsMediaUriEndPointConstructorParams (std::string uri)
throw (KmsMediaServerException)
{
  std::map<std::string, KmsMediaParam> params;
  KmsMediaParam param;
  KmsMediaUriEndPointConstructorParams uriEpParams;

  uriEpParams.__set_uri (uri);
  param.__set_dataType (g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  param.__set_data (marshalKmsMediaUriEndPointConstructorParams (uriEpParams) );
  params[g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE] = param;

  return params;
}

std::string
marshalKmsMediaUriEndPointConstructorParams (KmsMediaUriEndPointConstructorParams &uriEpParams)
throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);
  std::string data;

  try {
    uriEpParams.write (&protocol);
    transport->appendBufferToString (data);
  } catch (...) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,
        "Cannot marshal KmsMediaUriEndPointConstructorParams");
  }

  return data;
}

std::shared_ptr<KmsMediaUriEndPointConstructorParams>
unmarshalKmsMediaUriEndPointConstructorParams (std::string data)
throw (KmsMediaServerException)
{
  std::shared_ptr<KmsMediaUriEndPointConstructorParams> params (new KmsMediaUriEndPointConstructorParams () );
  boost::shared_ptr<TMemoryBuffer> transport;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    params->read (&protocol);
  } catch (...) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
        "Cannot unmarshal KmsMediaUriEndPointConstructorParams");
  }

  return params;
}

} // kurento
