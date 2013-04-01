/*
 * MediaSrc.cpp - Kurento Media Server
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

#include "MediaSrc.hpp"
#include "Joinable.hpp"
#include <gst/gst.h>

#define GST_CAT_DEFAULT media_src
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "MediaSrc"

namespace kurento
{

MediaSrc::MediaSrc (Joinable *joinable) : MediaObjectImpl (joinable->token)
{
  this->joinable = joinable;
}

MediaSrc::~MediaSrc() throw ()
{

}

MediaType::type
MediaSrc::getMediaType ()
{
  return MediaType::AUDIO;
}

void
MediaSrc::connect (const MediaSink &mediaSink)
{
  //TODO: complete
  GST_INFO ("connect %ld to %ld", this->id, mediaSink.id);
}

void
MediaSrc::disconnect (const MediaSink &mediaSink)
{
  //TODO: complete
  GST_INFO ("disconnect %ld from %ld", this->id, mediaSink.id);
}

std::vector < std::shared_ptr<MediaSink> > *
MediaSrc:: getConnectedSinks ()
{
  std::vector< std::shared_ptr<MediaSink> > *mediaSinks = new std::vector< std::shared_ptr<MediaSink> >();

  //TODO: complete
  mediaSinks->push_back (std::shared_ptr<MediaSink> (new MediaSink (joinable) ) );

  return mediaSinks;
}

MediaSrc::StaticConstructor MediaSrc::staticConstructor;

MediaSrc::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
