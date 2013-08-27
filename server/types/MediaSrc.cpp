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

static void
pad_unlinked (GstPad *pad, GstPad *peer, GstElement *parent)
{
  gst_element_release_request_pad (parent, peer);
}

void
MediaSrc::connect (std::shared_ptr<MediaSink> mediaSink)
{
  GstPad *pad;

  GST_INFO ("connect %ld to %ld", this->id, mediaSink->id);

  mutex.lock();

  pad = gst_element_get_request_pad (getElement(), getPadName().c_str() );

  g_signal_connect (G_OBJECT (pad), "unlinked", G_CALLBACK (pad_unlinked), getElement() );

  if (mediaSink->linkPad (shared_from_this(), pad) ) {
    connectedSinks.push_back (std::weak_ptr<MediaSink> (mediaSink) );
  } else {
    gst_element_release_request_pad (getElement(), pad);
    GST_WARNING ("Cannot connect %ld to %ld", this->id, mediaSink->id);
  }

  g_object_unref (pad);

  mutex.unlock();
}

void
MediaSrc::removeSink (MediaSink *mediaSink)
{
  std::shared_ptr<MediaSink> sinkLocked;
  std::vector< std::weak_ptr<MediaSink> >::iterator it;

  mutex.lock();

  it = connectedSinks.begin();

  while (it != connectedSinks.end() ) {
    try {
      sinkLocked = (*it).lock();
    } catch (std::bad_weak_ptr e) {
    }

    if (sinkLocked == NULL || sinkLocked->id == mediaSink->id) {
      it = connectedSinks.erase (it);
    } else {
      it++;
    }
  }

  mutex.unlock();
}

void
MediaSrc::disconnect (std::shared_ptr<MediaSink> mediaSink)
{
  MediaSrc::disconnect (mediaSink.get () );
}

void
MediaSrc::disconnect (MediaSink *mediaSink)
{
  GST_INFO ("disconnect %ld from %ld", this->id, mediaSink->id);

  mutex.lock();

  mediaSink->unlink (shared_from_this(), NULL);

  mutex.unlock();
}

std::vector < std::shared_ptr<MediaSink> > *
MediaSrc:: getConnectedSinks ()
{
  std::shared_ptr<MediaSink> sinkLocked;
  std::vector< std::weak_ptr<MediaSink> >::iterator it;

  std::vector< std::shared_ptr<MediaSink> > *mediaSinks = new std::vector< std::shared_ptr<MediaSink> >();

  mutex.lock();

  for ( it = connectedSinks.begin() ; it != connectedSinks.end(); ++it) {
    try {
      sinkLocked = (*it).lock();
    } catch (std::bad_weak_ptr e) {
    }

    if (sinkLocked != NULL) {
      mediaSinks->push_back (sinkLocked);
    }
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
