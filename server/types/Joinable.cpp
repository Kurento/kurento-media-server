/*
 * Joinable.cpp - Kurento Media Server
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

#include "Joinable.hpp"
#include <gst/gst.h>

#define GST_CAT_DEFAULT media_joinable
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "media_joinable"

namespace kurento
{

Joinable::Joinable (MediaFactory &mediaFactory) : MediaObjectImpl (mediaFactory.token)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
  this->mediaFactory = &mediaFactory;
}

Joinable::~Joinable () throw ()
{

}

void
Joinable::join (const Joinable &to)
{
  GST_INFO ("join %ld to %ld", this->id, to.id);
}

void
Joinable::unjoin (const Joinable &to)
{
  GST_INFO ("unjoin %ld to %ld", this->id, to.id);
}

std::vector < std::shared_ptr<MediaSrc> > *
Joinable::getMediaSrcs ()
{
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs = new std::vector< std::shared_ptr<MediaSrc> >();

  //TODO: complete
  mediaSrcs->push_back (std::shared_ptr<MediaSrc> (new MediaSrc (*mediaFactory) ) );

  return mediaSrcs;
}

std::vector < std::shared_ptr<MediaSink> > *
Joinable::getMediaSinks()
{
  std::vector< std::shared_ptr<MediaSink> > *mediaSinks = new std::vector< std::shared_ptr<MediaSink> >();

  //TODO: complete
  mediaSinks->push_back (std::shared_ptr<MediaSink> (new MediaSink (*mediaFactory) ) );

  return mediaSinks;
}

std::vector < std::shared_ptr<MediaSrc> > *
Joinable::getMediaSrcsByMediaType (const MediaType::type mediaType)
{
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs = new std::vector< std::shared_ptr<MediaSrc> >();

  //TODO: complete
  mediaSrcs->push_back (std::shared_ptr<MediaSrc> (new MediaSrc (*mediaFactory) ) );

  return mediaSrcs;
}

std::vector < std::shared_ptr<MediaSink> > *
Joinable::getMediaSinksByMediaType (const MediaType::type mediaType)
{
  std::vector< std::shared_ptr<MediaSink> > *mediaSinks = new std::vector< std::shared_ptr<MediaSink> >();

  //TODO: complete
  mediaSinks->push_back (std::shared_ptr<MediaSink> (new MediaSink (*mediaFactory) ) );

  return mediaSinks;
}

} // kurento
