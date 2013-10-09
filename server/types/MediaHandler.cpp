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
  ~MediaHandler ();

private:
  std::string eventType;
  std::string address;
  int32_t port;
  std::string callbackToken;

  friend class MediaHandlerManager;
};

/* MediaHandler */
MediaHandler::MediaHandler (const std::string eventType, const std::string &address, const int32_t port)
{
  this->eventType = eventType;
  this->address = address;
  this->port = port;
  this->callbackToken = generateUUID ();
}

MediaHandler::~MediaHandler ()
{
}

/* MediaHandlerManager */
MediaHandlerManager::MediaHandlerManager ()
{
}

MediaHandlerManager::~MediaHandlerManager ()
{
}

std::string
MediaHandlerManager::addMediaHandler (std::string eventType, const std::string &handlerAddress, const int32_t handlerPort)
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

  return mediaHandler->callbackToken;
}

void
MediaHandlerManager::removeMediaHandler (std::string callbackToken)
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
MediaHandlerManager::getMediaHandlersSetSize (std::string eventType)
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
