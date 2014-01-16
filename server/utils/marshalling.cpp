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

#include "KmsMediaObject_constants.h"
#include "KmsMediaUriEndPointType_constants.h"

namespace kurento
{

void
marshalI32 (std::string &_return,
            const int32_t i) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);

  try {
    protocol.writeI32 (i);
    transport->appendBufferToString (_return);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,
                                   "Cannot marshal I32");
    throw except;
  }
}

int32_t
unmarshalI32 (const std::string &data) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport;
  int32_t i;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (
                  uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    protocol.readI32 (i);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Cannot unmarshal I32");
    throw except;
  }

  return i;
}

void
marshalString (std::string &_return,
               const std::string &str) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);

  try {
    protocol.writeString (str);
    transport->appendBufferToString (_return);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,
                                   "Cannot marshal string");
    throw except;
  }
}

void
unmarshalString (std::string &_return,
                 const std::string &data) throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (
                  uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    protocol.readString (_return);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Cannot unmarshal string");
    throw except;
  }
}

void
createVoidParam (KmsMediaParam &_return) throw (KmsMediaServerException)
{
  _return.__set_dataType (g_KmsMediaDataType_constants.VOID_DATA_TYPE);
}

void
createI32Param (KmsMediaParam &_return,
                const int32_t i) throw (KmsMediaServerException)
{
  marshalI32 (_return.data, i);
  _return.__isset.data = true;

  _return.__set_dataType (g_KmsMediaDataType_constants.I32_DATA_TYPE);
}

int32_t
unmarshalI32Param (const KmsMediaParam &param) throw (KmsMediaServerException)
{
  int32_t i;

  if (g_KmsMediaDataType_constants.I32_DATA_TYPE.compare (param.dataType) == 0) {
    i = unmarshalI32 (param.data);
  } else {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Param is not of 'I32' data type");
    throw except;
  }

  return i;
}

void
createStringParam (KmsMediaParam &_return,
                   const std::string &data) throw (KmsMediaServerException)
{
  marshalString (_return.data, data);
  _return.__isset.data = true;

  _return.__set_dataType (g_KmsMediaDataType_constants.STRING_DATA_TYPE);
}

void
unmarshalStringParam (std::string &_return,
                      const KmsMediaParam &param) throw (KmsMediaServerException)
{
  if (g_KmsMediaDataType_constants.STRING_DATA_TYPE.compare (
        param.dataType) == 0) {
    unmarshalString (_return, param.data);
  } else {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Param is not of 'String' data type");
    throw except;
  }
}

void
createKmsMediaObjectConstructorParams (
  std::map<std::string, KmsMediaParam> &_return,
  bool excludeFromGC, int32_t garbageCollectorPeriod)
throw (KmsMediaServerException)
{
  KmsMediaParam param;
  KmsMediaObjectConstructorParams moParams;

  moParams.__set_excludeFromGC (excludeFromGC);

  if (garbageCollectorPeriod !=
      g_KmsMediaServer_constants.DEFAULT_GARBAGE_COLLECTOR_PERIOD) {
    moParams.__set_garbageCollectorPeriod (garbageCollectorPeriod);
  }

  param.__set_dataType (g_KmsMediaObject_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  marshalKmsMediaObjectConstructorParams (param.data, moParams);
  param.__isset.data = true;
  _return[g_KmsMediaObject_constants.CONSTRUCTOR_PARAMS_DATA_TYPE] = param;
}

void
marshalKmsMediaObjectConstructorParams (
  KmsMediaUriEndPointConstructorParams &_return, std::string &data)
throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (
                  uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    _return.read (&protocol);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Cannot unmarshal KmsMediaUriEndPointConstructorParams");
    throw except;
  }
}

void
marshalKmsMediaObjectConstructorParams (std::string &_return,
                                        KmsMediaObjectConstructorParams &moParams)
throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);

  try {
    moParams.write (&protocol);
    transport->appendBufferToString (_return);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,
                                   "Cannot marshal KmsMediaObjectConstructorParams");
    throw except;
  }
}

void
unmarshalKmsMediaObjectConstructorParams (
  KmsMediaObjectConstructorParams &_return, const std::string &data)
throw (KmsMediaServerException)
{
  std::shared_ptr<KmsMediaObjectConstructorParams> params (
    new KmsMediaObjectConstructorParams () );
  boost::shared_ptr<TMemoryBuffer> transport;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (
                  uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    _return.read (&protocol);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Cannot unmarshal KmsMediaObjectConstructorParams");
    throw except;
  }
}

void
createKmsMediaUriEndPointConstructorParams (std::map<std::string, KmsMediaParam>
    &_return, const std::string &uri)
throw (KmsMediaServerException)
{
  KmsMediaParam param;
  KmsMediaUriEndPointConstructorParams uriEpParams;

  uriEpParams.__set_uri (uri);
  param.__set_dataType (
    g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  marshalKmsMediaUriEndPointConstructorParams (param.data, uriEpParams);
  param.__isset.data = true;
  _return[g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE] =
    param;

}

void
marshalKmsMediaUriEndPointConstructorParams (std::string &_return,
    KmsMediaUriEndPointConstructorParams &uriEpParams)
throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);
  std::string data;

  try {
    uriEpParams.write (&protocol);
    transport->appendBufferToString (_return);
  } catch (...) {
    KmsMediaServerException except;
    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,
                                   "Cannot marshal KmsMediaUriEndPointConstructorParams");
    throw except;
  }
}

void
unmarshalKmsMediaUriEndPointConstructorParams (
  KmsMediaUriEndPointConstructorParams &_return, const std::string &data)
throw (KmsMediaServerException)
{
  boost::shared_ptr<TMemoryBuffer> transport;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (
                  uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    _return.read (&protocol);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Cannot unmarshal KmsMediaUriEndPointConstructorParams");
    throw except;
  }
}

} // kurento
