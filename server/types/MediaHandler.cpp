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

#include <gst/gst.h>
#include "utils/utils.hpp"

#define GST_CAT_DEFAULT kurento_media_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaHandler"

using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::protocol;

namespace kurento
{

class MediaHandler
{
public:
  MediaHandler (const std::string eventType, const std::string &address, const int32_t port);
  MediaHandler (const std::string &address, const int32_t port);
  ~MediaHandler ();

private:
  std::string eventType;
  std::string address;
  int32_t port;
  std::string callbackToken;

  friend class MediaHandlerManager;
  friend void send_to_client (gpointer p, gpointer data);
};

/* MediaHandler */
MediaHandler::MediaHandler (const std::string eventType, const std::string &address, const int32_t port)
{
  this->eventType = eventType;
  this->address = address;
  this->port = port;
  generateUUID (callbackToken);
}

MediaHandler::MediaHandler (const std::string &address, const int32_t port)
{
  this->address = address;
  this->port = port;
  generateUUID (callbackToken);
}

MediaHandler::~MediaHandler ()
{
}

/* MediaHandlerManager */

typedef struct _SendData {
  std::shared_ptr<MediaHandler> mediaHandler;
  bool is_error;
  union {
    std::shared_ptr<KmsMediaEvent> event;
    std::shared_ptr<KmsMediaError> error;
  };
} SendData;

void
send_to_client (gpointer data, gpointer user_data)
{
  SendData *d = (SendData *) data;
  std::shared_ptr<MediaHandler> mh = d->mediaHandler;

  boost::shared_ptr<TSocket> socket (new TSocket (mh->address, mh->port) );
  boost::shared_ptr<TTransport> transport (new TFramedTransport (socket) );
  boost::shared_ptr<TBinaryProtocol> protocol (new TBinaryProtocol (transport) );
  KmsMediaHandlerServiceClient client (protocol);

  try {
    transport->open();

    if (d->is_error)
      client.onError (mh->callbackToken, * (d->error) );
    else
      client.onEvent (mh->callbackToken, * (d->event) );

    transport->close();
  } catch (...) {
    GST_WARNING ("Error sending event to MediaHandler(%s, %s:%d)",
                 mh->callbackToken.c_str (), mh->address.c_str (), mh->port);
  }

  g_slice_free (SendData, data);
}

MediaHandlerManager::MediaHandlerManager ()
{
  threadPool = g_thread_pool_new (send_to_client, NULL, -1, FALSE, NULL);
}

MediaHandlerManager::~MediaHandlerManager ()
{
  g_thread_pool_free (threadPool, TRUE, FALSE);
}

void
MediaHandlerManager::addMediaHandler (std::string &_return,
                                      const std::string &eventType,
                                      const std::string &handlerAddress,
                                      const int32_t handlerPort)
{
  std::map<std::string, std::shared_ptr<std::set<std::shared_ptr<MediaHandler>> > >::iterator it;
  std::shared_ptr<std::set<std::shared_ptr<MediaHandler>>> handlers;
  std::shared_ptr<MediaHandler> mediaHandler (new MediaHandler (eventType, handlerAddress, handlerPort) );

  mutex.lock ();
  it = eventTypesMap.find (eventType);

  if (it != eventTypesMap.end() ) {
    handlers = it->second;
  } else {
    handlers = std::shared_ptr<std::set<std::shared_ptr<MediaHandler>>> (new std::set<std::shared_ptr<MediaHandler>>() );
    eventTypesMap[eventType] = handlers;
  }

  handlers->insert (mediaHandler);
  handlersMap[mediaHandler->callbackToken] = mediaHandler;
  mutex.unlock ();

  GST_DEBUG ("MediaHandler(%s, %s, %d) added for '%s' type event", mediaHandler->callbackToken.c_str (),
             handlerAddress.c_str (), handlerPort, eventType.c_str () );

  _return = mediaHandler->callbackToken;
}

void
MediaHandlerManager::removeMediaHandler (const std::string &callbackToken)
{
  std::map < std::string /*callbackToken*/, std::shared_ptr<MediaHandler >>::iterator handlersMapIt;
  std::map < std::string /*eventType*/, std::shared_ptr<std::set<std::shared_ptr<MediaHandler>> > >::iterator eventTypesMapIt;
  std::shared_ptr<std::set<std::shared_ptr<MediaHandler>>> handlers;
  std::shared_ptr<MediaHandler> mediaHandler;

  mutex.lock ();
  handlersMapIt = handlersMap.find (callbackToken);

  if (handlersMapIt == handlersMap.end () ) {
    GST_WARNING ("MediaHandler with '%s' callbackToken not found", callbackToken.c_str () );
    goto end;
  }

  mediaHandler = handlersMapIt->second;
  handlersMap.erase (callbackToken);

  eventTypesMapIt = eventTypesMap.find (mediaHandler->eventType);

  if (eventTypesMapIt != eventTypesMap.end() ) {
    handlers = eventTypesMapIt->second;
    handlers->erase (mediaHandler);
  }

end:
  mutex.unlock ();
}

void
MediaHandlerManager::sendEvent (std::shared_ptr<KmsMediaEvent> event)
{
  std::map < std::string /*eventType*/, std::shared_ptr<std::set<std::shared_ptr<MediaHandler>> >>::iterator eventTypesMapIt;
  std::set<std::shared_ptr<MediaHandler>>::iterator mediaHandlerIt;
  std::shared_ptr<std::set<std::shared_ptr<MediaHandler>> > handlersCopy;
  sigc::slot<void, std::shared_ptr<MediaHandler>, KmsMediaEvent> s;

  mutex.lock();
  eventTypesMapIt = eventTypesMap.find (event->type);

  if (eventTypesMapIt == eventTypesMap.end () ) {
    mutex.unlock();
    return;
  }

  handlersCopy = eventTypesMapIt->second;
  mutex.unlock();

  mediaHandlerIt = handlersCopy->begin ();

  for (; mediaHandlerIt != handlersCopy->end(); ++mediaHandlerIt) {
    SendData *data = g_slice_new0 (SendData);

    data->mediaHandler = *mediaHandlerIt;
    data->is_error = false;
    data->event = event;
    g_thread_pool_push (threadPool, data, NULL);
  }
}

void
MediaHandlerManager::addMediaErrorHandler (std::string &_return,
    const std::string &handlerAddress,
    const int32_t handlerPort)
{
  std::shared_ptr<MediaHandler> mediaHandler (new MediaHandler (handlerAddress,
      handlerPort) );

  mutex.lock ();
  errorHandlersMap[mediaHandler->callbackToken] = mediaHandler;
  mutex.unlock ();

  GST_DEBUG ("MediaHandler(%s, %s, %d) added for error",
             mediaHandler->callbackToken.c_str (),
             handlerAddress.c_str (), handlerPort);

  _return = mediaHandler->callbackToken;
}

void
MediaHandlerManager::removeMediaErrorHandler (const std::string &callbackToken)
{
  mutex.lock ();
  errorHandlersMap.erase (callbackToken);
  mutex.unlock ();
}

void
MediaHandlerManager::sendError (std::shared_ptr<KmsMediaError> error)
{
  mutex.lock();

  for (auto it = errorHandlersMap.begin(); it != errorHandlersMap.end(); it++) {
    SendData *data = g_slice_new0 (SendData);

    data->mediaHandler = it->second;
    data->is_error = true;
    data->error = error;
    g_thread_pool_push (threadPool, data, NULL);
  }

  mutex.unlock();
}

int
MediaHandlerManager::getHandlersMapSize ()
{
  int size;

  mutex.lock ();
  size = handlersMap.size ();
  mutex.unlock ();

  return size;
}

int
MediaHandlerManager::getEventTypesMapSize ()
{
  int size;

  mutex.lock ();
  size = eventTypesMap.size ();
  mutex.unlock ();

  return size;
}

int
MediaHandlerManager::getMediaHandlersSetSize (const std::string &eventType)
{
  int size;
  std::map<std::string, std::shared_ptr<std::set<std::shared_ptr<MediaHandler>> > >::iterator it;
  std::shared_ptr<std::set<std::shared_ptr<MediaHandler>>> handlers;

  mutex.lock ();
  it = eventTypesMap.find (eventType);

  if (it != eventTypesMap.end() ) {
    handlers = it->second;
    size = handlers->size ();
  } else {
    size = 0;
  }

  mutex.unlock ();

  return size;
}

MediaHandlerManager::StaticConstructor MediaHandlerManager::staticConstructor;

MediaHandlerManager::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

}
