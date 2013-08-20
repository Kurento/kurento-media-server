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

#include "MediaElement.hpp"

#define GST_CAT_DEFAULT kurento_media_src
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSrc"

namespace kurento
{

MediaSrc::MediaSrc (std::shared_ptr< kurento::MediaElement > parent, kurento::MediaType::type mediaType) : MediaPad (parent, mediaType, MediaPadType::type::MEDIA_SRC)
{

}

MediaSrc::~MediaSrc() throw ()
{

}

std::string
MediaSrc::getPadName ()
{
  if (getMediaType() == MediaType::type::AUDIO)
    return "audio_src_%u";
  else
    return "video_src_%u";
}

void
MediaSrc::connect (std::shared_ptr<MediaSink> mediaSink)
{
  GstPad *pad;

  mutex.lock();

  pad = gst_element_get_request_pad (getElement(), getPadName().c_str() );
  gst_element_release_request_pad (getElement(), pad);

  if (mediaSink->linkPad (shared_from_this(), pad) ) {
    connectedSinks.insert (mediaSink);
  }

  mutex.unlock();

  GST_INFO ("connect %ld to %ld", this->id, mediaSink->id);
}

void
MediaSrc::removeSink (std::shared_ptr<MediaSink> mediaSink)
{
  mutex.lock();

  connectedSinks.erase (mediaSink);

  mutex.unlock();
}

void
MediaSrc::disconnect (std::shared_ptr<MediaSink> mediaSink)
{
  mutex.lock();

  mediaSink->unlink (shared_from_this(), NULL);

  mutex.unlock();

  GST_INFO ("disconnect %ld from %ld", this->id, mediaSink->id);
}

std::vector < std::shared_ptr<MediaSink> > *
MediaSrc:: getConnectedSinks ()
{
  std::set< std::shared_ptr<MediaSink> >::iterator it;

  std::vector< std::shared_ptr<MediaSink> > *mediaSinks = new std::vector< std::shared_ptr<MediaSink> >();

  mutex.lock();

  for ( it = connectedSinks.begin() ; it != connectedSinks.end(); ++it) {
    mediaSinks->push_back (*it);
  }

  mutex.unlock();

  return mediaSinks;
}

MediaSrc::StaticConstructor MediaSrc::staticConstructor;

MediaSrc::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
