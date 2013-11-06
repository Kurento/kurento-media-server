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

#include "KmsMediaPointerDetectorFilterType_types.h"
#include "KmsMediaErrorCodes_constants.h"

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"
#include "protocol/TProtocol.h"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

void marshalI32 (std::string &_return, const int32_t i) throw (KmsMediaServerException);
int32_t unmarshalI32 (const std::string &data) throw (KmsMediaServerException);
void marshalString (std::string &_return, const std::string &str) throw (KmsMediaServerException);
void unmarshalString (std::string &_return, const std::string &data) throw (KmsMediaServerException);
#define unmarshalStruct(_return, msData)                                \
do {                                                                    \
  boost::shared_ptr<TMemoryBuffer> transport;                           \
  try {                                                                 \
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer     \
                    ( (uint8_t *) msData.data(), msData.size () ) );    \
    TBinaryProtocol protocol = TBinaryProtocol (transport);             \
    _return.read(&protocol);                                            \
  } catch (...) {                                                       \
    KmsMediaServerException except;                                     \
    createKmsMediaServerException (except,                              \
        g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,                \
        "Cannot unmarshal KmsMediaPointerDetectorWindowSet");           \
    throw except;                                                       \
  }                                                                     \
} while (0)
#define marshalStruct(_return, st)                                      \
do {                                                                    \
  try {                                                                 \
    boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );  \
    TBinaryProtocol protocol (transport);                               \
    st.write(&protocol);                                                \
    _return.clear();                                                    \
    transport->appendBufferToString (_return);                          \
  } catch (...) {                                                       \
    KmsMediaServerException except;                                     \
                                                                        \
    createKmsMediaServerException (except,                              \
        g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,                  \
        "Cannot marshal KmsMediaPointerDetectorWindow");                \
    throw except;                                                       \
  }                                                                     \
} while (0)

void createVoidParam (KmsMediaParam &_return) throw (KmsMediaServerException);
void createI32Param (KmsMediaParam &_return, const int32_t i) throw (KmsMediaServerException);
int32_t unmarshalI32Param (const KmsMediaParam &param) throw (KmsMediaServerException);
void createStringParam (KmsMediaParam &_return, const std::string &data) throw (KmsMediaServerException);
void unmarshalStringParam (std::string &_return, const KmsMediaParam &param) throw (KmsMediaServerException);

#define createStructParam(_return, st, type)                            \
do {                                                                    \
  try {                                                                 \
    _return.__set_dataType (type);                                      \
    marshalStruct (_return.data, st);                                   \
    _return.__isset.data = true;                                        \
  } catch (...) {                                                       \
    KmsMediaServerException except;                                     \
                                                                        \
    createKmsMediaServerException (except,                              \
        g_KmsMediaErrorCodes_constants.MARSHALL_ERROR,                  \
        "Cannot create struct param");                                  \
    throw except;                                                       \
  }                                                                     \
} while (0)

inline void createVoidInvocationReturn (KmsMediaInvocationReturn &_return)
throw (KmsMediaServerException)
{
  createVoidParam ((KmsMediaParam &) _return);
}

inline void createStringInvocationReturn (KmsMediaInvocationReturn &_return, const std::string &data)
throw (KmsMediaServerException)
{
  createStringParam ( (KmsMediaParam &) _return, data);
}

inline void unmarshalStringInvocationReturn (std::string &_return,
    const KmsMediaInvocationReturn &invocationReturn)
throw (KmsMediaServerException)
{
  unmarshalStringParam (_return, (KmsMediaParam) invocationReturn);
}

inline void createStringEventData (KmsMediaEventData &_return, const std::string &data) throw (KmsMediaServerException)
{
  createStringParam ( (KmsMediaParam &) _return, data);
}

inline void unmarshalStringEventData (std::string &_return, const KmsMediaEventData &eventData)
throw (KmsMediaServerException)
{
  unmarshalStringParam (_return, (KmsMediaEventData) eventData);
}

void createKmsMediaObjectConstructorParams (std::map<std::string, KmsMediaParam> & _return,
    bool excludeFromGC, int32_t garbageCollectorPeriod = g_KmsMediaServer_constants.DEFAULT_GARBAGE_COLLECTOR_PERIOD)
throw (KmsMediaServerException);
void marshalKmsMediaObjectConstructorParams (std::string &_return, KmsMediaObjectConstructorParams &moParams)
throw (KmsMediaServerException);
void unmarshalKmsMediaObjectConstructorParams (KmsMediaObjectConstructorParams &_return, const std::string &data)
throw (KmsMediaServerException);

void createKmsMediaUriEndPointConstructorParams (std::map<std::string, KmsMediaParam> & _return,
    const std::string &uri)
throw (KmsMediaServerException);
void marshalKmsMediaUriEndPointConstructorParams (std::string &_return,
    KmsMediaUriEndPointConstructorParams &uriEpParams)
throw (KmsMediaServerException);
void unmarshalKmsMediaUriEndPointConstructorParams (
  KmsMediaUriEndPointConstructorParams &_return, const std::string &data)
throw (KmsMediaServerException);

} // kurento

#endif /* __MARSHALLING_HPP__ */
