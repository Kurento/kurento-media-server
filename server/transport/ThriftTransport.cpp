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

#include <gst/gst.h>
#include "ThriftTransport.hpp"

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include "ThriftTransportHandler.hpp"

#define GST_CAT_DEFAULT kurento_thrift_transport
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoThriftTransport"

#define DEFAULT_ADDRESS "localhost"
#define DEFAULT_PORT 9090

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

namespace kurento
{

static void
check_port (int port)
{
  if (port <= 0 || port > G_MAXUSHORT) {
    throw boost::property_tree::ptree_bad_data ("Invalid value for port", port);
  }
}

ThriftTransport::ThriftTransport (const boost::property_tree::ptree &config) :
  Transport (config), config (config)
{
  try {
    port = config.get<uint> ("mediaServer.netInterface.thrift");
    check_port (port);
  } catch (const boost::property_tree::ptree_error &e) {
    GST_WARNING ("Setting default port %d to media server",
                 DEFAULT_PORT);
    port = DEFAULT_PORT;
  }
}

void ThriftTransport::serve()
{
  shared_ptr < ThriftTransportHandler > handler (new
      ThriftTransportHandler (config) );
  shared_ptr < TProcessor > processor (new KmsMediaServerServiceProcessor (
                                         handler) );
  shared_ptr < TProtocolFactory > protocolFactory (new
      TBinaryProtocolFactory () );
  shared_ptr < PosixThreadFactory > threadFactory (new PosixThreadFactory () );
  shared_ptr < ThreadManager > threadManager =
    ThreadManager::newSimpleThreadManager (15);
  threadManager->threadFactory (threadFactory);
  threadManager->start ();
  server = std::shared_ptr<apache::thrift::server::TNonblockingServer>
           (new apache::thrift::server::TNonblockingServer (processor, protocolFactory,
               port, threadManager) );

  GST_INFO ("Starting thrift server");
  server->serve ();
  GST_INFO ("Thrift server stopped");
}

void ThriftTransport::start ()
{
  GST_DEBUG ("Starting transport...");

  thread = Glib::Thread::create (std::bind  (&ThriftTransport::serve, this),
                                 true);
}

void ThriftTransport::stop ()
{
  GST_DEBUG ("stopping transport...");

  server->stop();
  thread->join();
}

ThriftTransport::~ThriftTransport()
{
  GST_DEBUG ("Destroying ThriftTransport");
}

ThriftTransport::StaticConstructor ThriftTransport::staticConstructor;

ThriftTransport::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
