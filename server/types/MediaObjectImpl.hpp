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

#ifndef __MEDIA_OBJECT_IMPL_HPP__
#define __MEDIA_OBJECT_IMPL_HPP__

#include "KmsMediaServer_types.h"
#include "KmsMediaServer_constants.h"
#include <gst/gst.h>

#include "MediaHandler.hpp"

namespace kurento
{

class MediaObjectImpl : public KmsMediaObjectRef
{
public:
  MediaObjectImpl (const std::map<std::string, KmsMediaParam>& params = emptyParams);
  MediaObjectImpl (std::shared_ptr<MediaObjectImpl> parent, const std::map<std::string, KmsMediaParam>& params = emptyParams);
  virtual ~MediaObjectImpl() throw () = 0;

  bool getExcludeFromGC ();

  std::shared_ptr<MediaObjectImpl> getParent () throw (KmsMediaServerException);
  virtual void invoke (KmsMediaInvocationReturn &_return, const std::string &command,
                       const std::map<std::string, KmsMediaParam> & params) throw (KmsMediaServerException);
  virtual void subscribe (std::string &_return, const std::string &eventType, const std::string &handlerAddress,
                          const int32_t handlerPort) throw (KmsMediaServerException);
  virtual void unsubscribe (const std::string &callbackToken) throw (KmsMediaServerException);

  virtual void subscribeError (std::string &_return,
                               const std::string &handlerAddress,
                               const int32_t handlerPort) throw (KmsMediaServerException);
  virtual void unsubscribeError (const std::string &callbackToken) throw (KmsMediaServerException);

public:
  std::shared_ptr<MediaObjectImpl> parent;
  int32_t getGarbageCollectorPeriod () {
    return garbageCollectorPeriod;
  }

protected:
  static std::map<std::string, KmsMediaParam> emptyParams;
  MediaHandlerManager mediaHandlerManager;

  void sendEvent (const std::string &eventType, const KmsMediaEventData &eventData = defaultKmsMediaEventData);
  void sendError (const std::string &errorType, const std::string &description,
                  int32_t errorCode);

private:
  bool excludeFromGC = false;
  int32_t garbageCollectorPeriod = g_KmsMediaServer_constants.DEFAULT_GARBAGE_COLLECTOR_PERIOD;

  void init (const std::map<std::string, KmsMediaParam>& params);
  void sendError (const std::shared_ptr<KmsMediaError> &error);

  static KmsMediaEventData defaultKmsMediaEventData;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __MEDIA_OBJECT_IMPL_HPP__ */
