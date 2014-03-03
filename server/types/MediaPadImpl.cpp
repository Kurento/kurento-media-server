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

#include "MediaPadImpl.hpp"
#include "MediaElementImpl.hpp"

namespace kurento
{

MediaPadImpl::MediaPadImpl (std::shared_ptr< MediaType > mediaType,
                            const std::string &mediaDescription,
                            std::shared_ptr< MediaObjectImpl > parent) :
  MediaObjectImpl (parent, 0),
  mediaType (mediaType),
  mediaDescription (mediaDescription)
{
}

std::shared_ptr<MediaElement>
MediaPadImpl::getMediaElement ()
{
  return std::dynamic_pointer_cast <MediaElement> (getParent() );
}

GstElement *
MediaPadImpl::getGstreamerElement ()
{
  std::shared_ptr<MediaElementImpl> element = std::dynamic_pointer_cast
      <MediaElementImpl> (getParent() );

  return element->getGstreamerElement();
}

std::shared_ptr<MediaType>
MediaPadImpl::getMediaType ()
{
  return mediaType;
}

std::string
MediaPadImpl::getMediaDescription ()
{
  return mediaDescription;
}

} /* kurento */
