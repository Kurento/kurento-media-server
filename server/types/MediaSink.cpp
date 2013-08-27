/*
 * MediaSink.cpp - Kurento Media Server
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

#include "MediaSink.hpp"
#include "MediaElement.hpp"

#define GST_CAT_DEFAULT kurento_media_sink
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSink"

namespace kurento
{

MediaSink::MediaSink (std::shared_ptr<MediaElement> parent, MediaType::type mediaType) : MediaPad (parent, mediaType, MediaPadType::type::MEDIA_SINK)
{

}

MediaSink::~MediaSink() throw ()
{
  std::shared_ptr<MediaSrc> connectedSrcLocked;

  try {
    connectedSrcLocked = connectedSrc.lock();
  } catch (std::bad_weak_ptr e) {
  }

  if (connectedSrcLocked != NULL) {
    connectedSrcLocked->disconnect (this);
  }
}

std::string
MediaSink::getPadName ()
{
  if (getMediaType() == MediaType::type::AUDIO)
    return "audio_sink";
  else
    return "video_sink";
}

bool
MediaSink::linkPad (std::shared_ptr<MediaSrc> mediaSrc, GstPad *src)
{
  std::shared_ptr<MediaSrc> connectedSrcLocked;
  GstPad *sink;
  bool ret;

  mutex.lock();

  try {
    connectedSrcLocked = connectedSrc.lock();
  } catch (std::bad_weak_ptr e) {
  }

  if ( (sink = gst_element_get_static_pad (getElement(), getPadName().c_str() ) ) == NULL)
    sink = gst_element_get_request_pad (getElement(), getPadName().c_str() );

  if (gst_pad_is_linked (sink) ) {
    unlink (connectedSrcLocked, sink);
  }

  if (gst_pad_link (src, sink) == GST_PAD_LINK_OK) {
    ret = true;
    connectedSrc = std::weak_ptr<MediaSrc> (mediaSrc);
  } else {
    gst_element_release_request_pad (getElement(), sink);
    ret = false;
  }

  g_object_unref (sink);

  mutex.unlock();

  return ret;
}

void
MediaSink::unlink (std::shared_ptr<MediaSrc> mediaSrc, GstPad *sink)
{
  std::shared_ptr<MediaSrc> connectedSrcLocked;

  mutex.lock();

  try {
    connectedSrcLocked = connectedSrc.lock();
  } catch (std::bad_weak_ptr e) {
  }

  if (connectedSrcLocked != NULL && mediaSrc == connectedSrcLocked) {
    GstPad *peer;
    GstPad *sinkPad;

    if (sink == NULL)
      sinkPad = gst_element_get_static_pad (getElement(), getPadName().c_str() );
    else
      sinkPad = sink;

    if (sinkPad == NULL)
      goto end;

    peer = gst_pad_get_peer (sinkPad);

    if (peer != NULL) {
      gst_pad_unlink (peer, sinkPad);

      g_object_unref (peer);
    }

    if (sink == NULL) {
      GstElement *elem;

      elem = gst_pad_get_parent_element (sinkPad);
      gst_element_release_request_pad (elem, sinkPad);
      g_object_unref (elem);
      g_object_unref (sinkPad);
    }

end:

    connectedSrcLocked->removeSink (this);
  }

  mutex.unlock();
}

std::shared_ptr<MediaSrc>
MediaSink::getConnectedSrc ()
{
  std::shared_ptr<MediaSrc> connectedSrcLocked;

  try {
    connectedSrcLocked = connectedSrc.lock();
  } catch (std::bad_weak_ptr e) {
  }

  return connectedSrcLocked;
}

MediaSink::StaticConstructor MediaSink::staticConstructor;

MediaSink::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
