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
#include <signal.h>
#include <glibmm/thread.h>

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <server/TServer.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include "HandlerTest.hpp"

#include <gst/gst.h>

#define GST_CAT_DEFAULT _handler_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "handler_test"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

namespace kurento
{

HandlerTest::HandlerTest ()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

HandlerTest::~HandlerTest()
{
}

void
HandlerTest::start()
{
  boost::shared_ptr < HandlerTest > handler (shared_from_this() );
  boost::shared_ptr < TProcessor > processor (new KmsMediaHandlerServiceProcessor (handler) );
  boost::shared_ptr < TProtocolFactory >
  protocolFactory (new TBinaryProtocolFactory () );
  boost::shared_ptr < PosixThreadFactory > threadFactory (new PosixThreadFactory () );
  boost::shared_ptr <TTransportFactory> transportFactory (new TFramedTransportFactory () );
  boost::shared_ptr <TServerTransport> serverTransport (new TServerSocket (HANDLER_PORT) );

  server = boost::shared_ptr<TSimpleServer> (new TSimpleServer (processor, serverTransport, transportFactory, protocolFactory) );

  boost::shared_ptr<Thread> serverThread;
  serverThread = threadFactory->newThread (
                   boost::dynamic_pointer_cast<Runnable> (server) );

  serverThread->start();
}

void
HandlerTest::stop()
{
  if (server) {
    server->stop();
    server.reset();
    //TODO: There is a race condition that we have to solve. Sleep is only
    // a temporal solution.
    sleep (1);
  }
}


void
HandlerTest::onError (const std::string &callbackToken,
                      const KmsMediaError &error)
{
  mutex.lock();

  if (this->errorFunc) {
    if (error.type == this->waitError) {
      this->errorFunc (callbackToken, error);
    }
  } else {
    GST_INFO ("ClientHandler: Without error function");
  }

  GST_INFO ("ClientHandler: Error received");
  mutex.unlock();
}

void
HandlerTest::onEvent (const std::string &callbackToken,
                      const KmsMediaEvent &event)
{
  mutex.lock();

  if (this->eventFunc) {
    if (event.type == this->waitEvent) {
      this->eventFunc (callbackToken, event);
    }
  } else {
    GST_INFO ("ClientHandler: Without event function");
  }

  GST_INFO ("ClientHandler: Event received");
  mutex.unlock();
}

void
HandlerTest::setEventFunction (const std::function
                               <void (std::string, KmsMediaEvent) > &function,
                               std::string waitEvent)
{
  mutex.lock();
  this->waitEvent = waitEvent;
  this->eventFunc = function;
  mutex.unlock();
}

void
HandlerTest::deleteEventFunction()
{
  mutex.lock ();
  this->eventFunc = NULL;
  mutex.unlock ();
}

void
HandlerTest::setErrorFunction (const std::function
                               <void (std::string, KmsMediaError) > &function,
                               std::string waitError)
{
  mutex.lock ();
  this->waitError = waitError;
  this->errorFunc = function;
  mutex.unlock ();
}

void HandlerTest::deleteErrorFunction()
{
  mutex.lock ();
  this->errorFunc = NULL;
  mutex.unlock ();
}

} // kurento
