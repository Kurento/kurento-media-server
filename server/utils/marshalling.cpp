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
#include "KmsMediaErrorCodes_constants.h"

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

std::string
marshalString (std::string str) throw (KmsMediaServerException)
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
unmarshalString (std::string data) throw (KmsMediaServerException)
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

} // kurento
