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
#include <uuid/uuid.h>

/* 36-byte string (plus tailing '\0') */
#define UUID_STR_SIZE 37

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
  uuid_t uuid;
  char *uuid_str;
  std::string tk;

  uuid_str = (char *) g_malloc (UUID_STR_SIZE);
  uuid_generate (uuid);
  uuid_unparse (uuid, uuid_str);
  tk.append (uuid_str);
  g_free (uuid_str);

  id = getId();
  this->token = tk;
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

} // kurento
