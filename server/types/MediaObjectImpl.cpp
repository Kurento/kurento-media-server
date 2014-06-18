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
#include "MediaPipelineImpl.hpp"
#include <stdlib.h>
#include <time.h>
#include <glibmm.h>
#include <jsonrpc/JsonSerializer.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace kurento
{

std::string
MediaObjectImpl::createId()
{
  std::stringstream ss;
  boost::uuids::uuid uuid = boost::uuids::random_generator() ();

  ss << uuid;

  if (parent) {
    std::shared_ptr<MediaPipelineImpl> pipeline;

    pipeline = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );
    return pipeline->getId() + "/" +
           ss.str();
  } else {
    return ss.str();
  }
}

MediaObjectImpl::MediaObjectImpl ()
{
  id = createId();
}

MediaObjectImpl::MediaObjectImpl (std::shared_ptr<MediaObjectImpl> parent)
{
  this->parent = parent;

  id = createId();

  signalError.connect ([parent] (Error error) {
    parent->signalError (error);
  });
}

std::string MediaObjectImpl::getId ()
{
  return id;
}

void
Serialize (std::shared_ptr<MediaObject> &object, JsonSerializer &s)
{
  if (s.IsWriter) {
    std::shared_ptr<MediaObjectImpl> objectImpl;
    objectImpl = std::dynamic_pointer_cast <MediaObjectImpl> (object);

    if (objectImpl) {
      Json::Value v (objectImpl->getId() );

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
      Json::Value v (objectImpl.getId() );

      s.JsonValue = v;
    } catch (std::bad_cast &e) {
    }
  } else {
    // TODO: Find the object getting from mediaSet
  }
}

} /* kurento */
