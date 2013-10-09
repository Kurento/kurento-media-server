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

#ifndef __MEDIA_HANDLER_H__
#define __MEDIA_HANDLER_H__

#include "KmsMediaHandler_types.h"

#include <glibmm.h>

namespace kurento
{

class MediaHandler;

class MediaHandlerManager
{
public:
  MediaHandlerManager ();
  ~MediaHandlerManager ();

  std::string addMediaHandler (const std::string eventType, const std::string &handlerAddress, const int32_t handlerPort);
  void removeMediaHandler (const std::string callbackToken);
  void sendEvent (KmsMediaEvent &event);

  int getHandlersMapSize ();
  int getEventTypesMapSize ();
  int getMediaHandlersSetSize (std::string eventType);

private:
  Glib::Threads::RecMutex mutex;
  std::map<std::string /*callbackToken*/, std::shared_ptr<MediaHandler>> handlersMap;
  std::map<std::string /*eventType*/, std::shared_ptr<std::set<std::shared_ptr<MediaHandler>> >> eventTypesMap;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __MEDIA_HANDLER_H__ */
