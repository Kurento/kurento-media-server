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

#include "MediaObjectImpl.hpp"
#include <stdlib.h>
#include <time.h>
#include <glibmm.h>
#include <JsonSerializer.hpp>

namespace kurento
{

guint64
MediaObjectImpl::createId()
{
  static Glib::Mutex mutex;
  static int seed_initiated = 0;
  uint64_t ret;

  mutex.lock();

  if (!seed_initiated) {
    srand48 (time (NULL) );
    seed_initiated = 1;
  }

  ret = lrand48();
  mutex.unlock();

  return ret;
}

MediaObjectImpl::MediaObjectImpl (int garbagePeriod)
{
  this->garbagePeriod = garbagePeriod;
}

MediaObjectImpl::MediaObjectImpl (std::shared_ptr<MediaObjectImpl> parent,
                                  int garbagePeriod)
{
  this->garbagePeriod = garbagePeriod;
  this->parent = parent;

  signalError.connect ([parent] (Error error) {
    parent->signalError (error);
  });
}

std::string MediaObjectImpl::getIdStr ()
{
  return std::to_string (id);
}

} /* kurento */

using namespace kurento;

void
Serialize (std::shared_ptr<MediaObject> &object, JsonSerializer &s)
{
  if (s.IsWriter) {
    std::shared_ptr<MediaObjectImpl> objectImpl;
    objectImpl = std::dynamic_pointer_cast <MediaObjectImpl> (object);

    if (objectImpl) {
      Json::Value v (objectImpl->getIdStr() );

      s.JsonValue = v;
    }
  } else {
    // TODO: Find the object getting from mediaSet
  }
}

void
Serialize (MediaObject &object, JsonSerializer &s)
{
  if (s.IsWriter) {
    try {
      MediaObjectImpl &objectImpl = dynamic_cast<MediaObjectImpl &> (object);
      Json::Value v (objectImpl.getIdStr() );

      s.JsonValue = v;
    } catch (std::bad_cast &e) {
    }
  } else {
    // TODO: Find the object getting from mediaSet
  }
}
