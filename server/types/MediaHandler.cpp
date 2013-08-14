/*
 * MediaManager.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MediaHandler.hpp"

#include "thrift/transport/TSocket.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/protocol/TBinaryProtocol.h"

#include "MediaHandlerService.h"

using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::protocol;

namespace kurento
{

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

}
