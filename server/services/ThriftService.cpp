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
#include "ThriftService.hpp"

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include <MediaServerServiceHandler.hpp>

#define GST_CAT_DEFAULT kurento_thrift_service
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoThriftService"

#define THRIFT_GROUP "Thrift"
#define THRIFT_SERVER_SERVICE_PORT "serverPort"

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
    throw Glib::KeyFileError (Glib::KeyFileError::PARSE, "Invalid value");
  }
}

ThriftService::ThriftService (Glib::KeyFile &confFile) : Service (confFile)
{
  try {
    port = confFile.get_integer (THRIFT_GROUP, THRIFT_SERVER_SERVICE_PORT);
    check_port (port);
  } catch (const Glib::KeyFileError &err) {
    GST_WARNING ("Setting default port %d to media server",
                 DEFAULT_PORT);
    port = DEFAULT_PORT;
  }

  httpService = std::shared_ptr<HttpService> (new HttpService (confFile) );
}

void ThriftService::serve()
{
  shared_ptr < MediaServerServiceHandler > handler (new
      MediaServerServiceHandler () );
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
  throw Glib::Thread::Exit ();
}

void ThriftService::start ()
{
  GST_DEBUG ("Starting service...");

  httpService->start();
  thread = Glib::Thread::create (std::bind  (&ThriftService::serve, this), true);
}

void ThriftService::stop ()
{
  GST_DEBUG ("stopping service...");

  server->stop();
  httpService->stop();
  thread->join();
}

ThriftService::~ThriftService()
{
  GST_DEBUG ("Destroying ThriftService");
}

ThriftService::StaticConstructor ThriftService::staticConstructor;

ThriftService::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
