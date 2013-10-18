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

#include "MediaSrc.hpp"

#include "MediaElement.hpp"

#define GST_CAT_DEFAULT kurento_media_src
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSrc"

namespace kurento
{

MediaSrc::MediaSrc (std::shared_ptr< kurento::MediaElement > parent, kurento::KmsMediaType::type mediaType)
  : MediaPad (parent, KmsMediaPadDirection::SRC, mediaType)
{

}

MediaSrc::~MediaSrc() throw ()
{

}

const gchar *
MediaSrc::getPadName ()
{
  if (mediaType == KmsMediaType::type::AUDIO)
    return (const gchar *) "audio_src_%u";
  else
    return (const gchar *) "video_src_%u";
}

static void
pad_unlinked (GstPad *pad, GstPad *peer, GstElement *parent)
{
  gst_element_release_request_pad (parent, pad);
}

void
MediaSrc::connect (std::shared_ptr<MediaSink> mediaSink)
{
  GstPad *pad;

  GST_INFO ("connect %" G_GINT64_FORMAT " to %" G_GINT64_FORMAT, this->id, mediaSink->id);

  mutex.lock();

  pad = gst_element_get_request_pad (getElement(), getPadName() );

  g_signal_connect (G_OBJECT (pad), "unlinked", G_CALLBACK (pad_unlinked), getElement() );

  if (mediaSink->linkPad (shared_from_this(), pad) ) {
    connectedSinks.push_back (std::weak_ptr<MediaSink> (mediaSink) );
  } else {
    gst_element_release_request_pad (getElement(), pad);
    GST_WARNING ("Cannot connect %" G_GINT64_FORMAT " to %" G_GINT64_FORMAT, this->id, mediaSink->id);
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
  GST_INFO ("disconnect %" G_GINT64_FORMAT " from %" G_GINT64_FORMAT, this->id, mediaSink->id);

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
