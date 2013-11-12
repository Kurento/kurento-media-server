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
#include <server/TNonblockingServer.h>
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
using boost::shared_ptr;

namespace kurento
{
static void
*thread_func (HandlerTest *handlerTest)
{
  shared_ptr < HandlerTest > handler (handlerTest);
  shared_ptr < TProcessor >
  processor (new KmsMediaHandlerServiceProcessor (handler) );
  shared_ptr < TProtocolFactory >
  protocolFactory (new TBinaryProtocolFactory () );
  shared_ptr < PosixThreadFactory > threadFactory (new PosixThreadFactory () );
  shared_ptr < ThreadManager > threadManager =
    ThreadManager::newSimpleThreadManager (1);
  threadManager->threadFactory (threadFactory);
  threadManager->start ();
  TNonblockingServer server (processor, protocolFactory, HANDLER_PORT,
                             threadManager);

  GST_INFO ("Starting MediaHandlerService");
  server.serve ();

  return NULL;
}

HandlerTest::HandlerTest ()
{
  GThread *th;

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
  GST_INFO ("ClientHandler: Handler Create");

  th = g_thread_new ("handler_server", (GThreadFunc) thread_func, this);
  g_thread_unref (th);
}

HandlerTest::~HandlerTest()
{
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
HandlerTest::setErrorFunction (const std::function
                               <void (std::string, KmsMediaError) > &function,
                               std::string waitError)
{
  mutex.lock ();
  this->waitError = waitError;
  this->errorFunc = function;
  mutex.unlock ();
}

} // kurento
