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

#include "MediaObjectImpl.hpp"

#include "utils/utils.hpp"
#include "KmsMediaErrorCodes_constants.h"

#include <glibmm.h>

namespace kurento
{

KmsMediaParams MediaObjectImpl::defaultKmsMediaParams = KmsMediaParams ();

static KmsMediaObjectId
getId()
{
  static Glib::Mutex mutex;
  static int seed_initiated = 0;
  int64_t ret;

  mutex.lock();

  if (!seed_initiated) {
    srand48 (time (NULL) );
    seed_initiated = 1;
  }

  ret = lrand48();
  mutex.unlock();

  return ret;
}

MediaObjectImpl::MediaObjectImpl() : KmsMediaObjectRef()
{
  id = getId();
  this->token = generateUUID ();
}

MediaObjectImpl::MediaObjectImpl (std::shared_ptr<MediaObjectImpl> parent) : KmsMediaObjectRef()
{
  id = getId();
  this->token = parent->token;
  this->parent = parent;
}

MediaObjectImpl::~MediaObjectImpl() throw ()
{
}

std::shared_ptr<MediaObjectImpl>
MediaObjectImpl::getParent () throw (KmsMediaServerException)
{
  if (parent == NULL) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_HAS_NOT_PARENT, "No parent");
  }

  return parent;
}

KmsMediaCommandResult
MediaObjectImpl::sendCommand (const KmsMediaCommand &command) throw (KmsMediaServerException)
{
  throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_COMMAND_NOT_FOUND,
      "This media object has not any command named " + command.name);
}

std::string
MediaObjectImpl::subscribe (const std::string &eventType, const std::string &handlerAddress, const int32_t handlerPort)
throw (KmsMediaServerException)
{
  throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_EVENT_NOT_SUPPORTED,
      "This media object has not any event named " + eventType);
}

void
MediaObjectImpl::unsubscribe (const std::string &callbackToken) throw (KmsMediaServerException)
{
  mediaHandlerManager.removeMediaHandler (callbackToken);
}

} // kurento
