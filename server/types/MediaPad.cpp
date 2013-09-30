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

#include "MediaPad.hpp"
#include "MediaElement.hpp"

#define GST_CAT_DEFAULT media_pad
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "MediaPad"

namespace kurento
{

class MediaElement;

MediaPad::MediaPad (std::shared_ptr<MediaElement> parent, MediaType::type mediaType, MediaPadType::type type) : MediaObjectImpl (parent)
{
  this->mediaType = mediaType;
  this->type.__set_mediaPad (type);
}

MediaPad::~MediaPad () throw ()
{

}

GstElement *
MediaPad::getElement ()
{
  return ( (std::shared_ptr<MediaElement> &) parent)->element;
}

MediaType::type
MediaPad::getMediaType ()
{
  return mediaType;
}

MediaPad::StaticConstructor MediaPad::staticConstructor;

MediaPad::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
