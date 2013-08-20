/*
 * MediaPad.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
