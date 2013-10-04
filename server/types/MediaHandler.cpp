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

#include "MediaHandler.hpp"

#include "thrift/transport/TSocket.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/protocol/TBinaryProtocol.h"

#include "KmsMediaHandlerService.h"

using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::protocol;

namespace kurento
{
// TODO: reuse when needed
#if 0
void
MediaHandler::sendEvent (MediaEvent &event)
{
  mutex.lock();
  std::list<std::shared_ptr<MediaHandlerAddress>>::iterator randIt = addresses.begin();
  std::advance (randIt, std::rand() % addresses.size() );
  std::shared_ptr<MediaHandlerAddress> addr = *randIt;
  mutex.unlock();

  boost::shared_ptr<TSocket> socket (new TSocket (addr->address, addr->port) );
  boost::shared_ptr<TTransport> transport (new TFramedTransport (socket) );
  boost::shared_ptr<TBinaryProtocol> protocol (new TBinaryProtocol (transport) );

  try {
    transport->open();

    MediaHandlerServiceClient client (protocol);

    client.onEvent (event);
    //TODO: Move the reply wating to a different thread to avoid locking main loop
    transport->close();
  } catch (...) {
    // TODO: Try to send event again or raise error;
  }
}
#endif
}
