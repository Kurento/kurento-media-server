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

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

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

std::shared_ptr<KmsMediaParams>
createStringParams (const std::string &data) throw (KmsMediaServerException)
{
  std::shared_ptr<KmsMediaParams> result (new KmsMediaParams () );

  result->__set_dataType (g_KmsMediaDataType_constants.STRING_DATA_TYPE);
  result->__set_data (marshalString (data) );

  return result;
}

std::string
unmarshalStringParams (const KmsMediaParams &params) throw (KmsMediaServerException)
{
  std::string str;

  if (g_KmsMediaDataType_constants.STRING_DATA_TYPE.compare (params.dataType) == 0) {
    str = unmarshalString (params.data);
  } else {
    throw createKmsMediaServerException  (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
        "Params is not of 'String' data type");
  }

  return str;
}

std::shared_ptr<KmsMediaCommand>
createVoidCommand (std::string commandName)
{
  std::shared_ptr<KmsMediaCommand> command (new KmsMediaCommand () );
  KmsMediaParams params;

  params.__set_dataType (g_KmsMediaDataType_constants.VOID_DATA_TYPE);
  command->__set_name (commandName);
  command->__set_params (params);

  return command;
}

std::shared_ptr<KmsMediaCommand>
createStringCommand (std::string commandName, std::string str)
throw (KmsMediaServerException)
{
  std::shared_ptr<KmsMediaCommand> command (new KmsMediaCommand () );
  KmsMediaParams params;

  params.__set_dataType (g_KmsMediaDataType_constants.STRING_DATA_TYPE);
  params.__set_data (marshalString (str) );
  command->__set_name (commandName);
  command->__set_params (params);

  return command;
}

} // kurento
