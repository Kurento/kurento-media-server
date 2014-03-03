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


#include "MediaSourceImpl.hpp"
#include "MediaSinkImpl.hpp"
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_media_source_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSourceImpl"

namespace kurento
{

struct tmp_data {
  std::weak_ptr<MediaSourceImpl> src;
  std::weak_ptr<MediaSinkImpl> sink;
  gulong handler;
};

static void
destroy_tmp_data (gpointer data, GClosure *closure)
{
  struct tmp_data *tmp = (struct tmp_data *) data;

  g_slice_free (struct tmp_data, tmp);
}

static struct tmp_data *
create_tmp_data (std::shared_ptr<MediaSourceImpl> src,
                 std::shared_ptr<MediaSinkImpl> sink)
{
  struct tmp_data *tmp;

  tmp = g_slice_new0 (struct tmp_data);

  tmp->src = std::weak_ptr<MediaSourceImpl> (src);
  tmp->sink = std::weak_ptr<MediaSinkImpl> (sink);
  tmp->handler = 0L;

  return tmp;
}

static void
pad_unlinked (GstPad *pad, GstPad *peer, GstElement *parent)
{
  gst_element_release_request_pad (parent, pad);
}

gboolean
link_media_elements (std::shared_ptr<MediaSourceImpl> src,
                     std::shared_ptr<MediaSinkImpl> sink)
{
  bool ret = FALSE;
  GstPad *pad;

  src->mutex.lock ();
  pad = gst_element_get_request_pad (src->getGstreamerElement(),
                                     src->getPadName() );

  if (pad == NULL) {
    goto end;
  }

  GST_WARNING ("Connecting pad %s", src->getPadName() );

  g_signal_connect (G_OBJECT (pad), "unlinked", G_CALLBACK (pad_unlinked),
                    src->getGstreamerElement() );

  if (sink->linkPad (src, pad) ) {
    src->connectedSinks.push_back (std::weak_ptr<MediaSinkImpl> (sink) );
    ret = TRUE;
  } else {
    gst_element_release_request_pad (src->getGstreamerElement(), pad);
    ret = FALSE;
  }

  gst_object_unref (pad);

end:
  src->mutex.unlock ();
  return ret;
}

static void
agnosticbin_added_cb (GstElement *element, gpointer data)
{
  struct tmp_data *tmp = (struct tmp_data *) data;
  std::shared_ptr<MediaSourceImpl> src;
  std::shared_ptr<MediaSinkImpl> sink;

  try {
    src = tmp->src.lock();
    sink = tmp->sink.lock();

    if (src && sink && link_media_elements (src, sink) ) {
      g_signal_handler_disconnect (element, tmp->handler);
    }
  } catch (const std::bad_weak_ptr &e) {
    GST_WARNING ("Removed before connecting");
    g_signal_handler_disconnect (element, tmp->handler);
  }
}

MediaSourceImpl::MediaSourceImpl (std::shared_ptr<MediaType> mediaType,
                                  const std::string &mediaDescription,
                                  std::shared_ptr<MediaObjectImpl> parent) :
  MediaPadImpl (mediaType, mediaDescription,
                parent)
{
}

MediaSourceImpl::~MediaSourceImpl()
{
  mutex.lock();

  for (auto it = connectedSinks.begin(); it != connectedSinks.end(); it++) {
    try {
      std::shared_ptr<MediaSinkImpl> connectedSinkLocked;

      GST_INFO ("connectedSink");
      connectedSinkLocked = it->lock();

      if (connectedSinkLocked != NULL) {
        connectedSinkLocked->unlinkUnchecked (NULL);
      }
    } catch (const std::bad_weak_ptr &e) {
      GST_WARNING ("Got invalid reference while releasing MediaSrc %"
                   G_GUINT64_FORMAT, getId() );
    }
  }

  mutex.unlock();
}

const gchar *
MediaSourceImpl::getPadName ()
{
  if ( ( (MediaPadImpl *) this)->getMediaType()->getValue() == MediaType::AUDIO) {
    return (const gchar *) "audio_src_%u";
  } else {
    return (const gchar *) "video_src_%u";
  }
}

void
MediaSourceImpl::connect (std::shared_ptr<MediaSink> mediaSink)
{
  std::shared_ptr<MediaSinkImpl> mediaSinkImpl =
    std::dynamic_pointer_cast<MediaSinkImpl> (mediaSink);
  GstPad *pad;
  bool ret;

  GST_INFO ("connect %" G_GUINT64_FORMAT " to %" G_GUINT64_FORMAT, this->getId(),
            mediaSinkImpl->getId() );

  mutex.lock();

  pad = gst_element_get_request_pad (getGstreamerElement(), getPadName() );

  if (pad == NULL) {
    struct tmp_data *tmp;

    GST_DEBUG ("Put connection off until agnostic bin is created for pad %s",
               getPadName() );
    tmp = create_tmp_data (shared_from_this(), mediaSinkImpl);
    tmp->handler = g_signal_connect_data (getGstreamerElement(),
                                          "agnosticbin-added",
                                          G_CALLBACK (agnosticbin_added_cb),
                                          tmp, destroy_tmp_data,
                                          (GConnectFlags) 0);

    mutex.unlock();
    return;
  }

  g_signal_connect (G_OBJECT (pad), "unlinked", G_CALLBACK (pad_unlinked),
                    getGstreamerElement() );

  ret = mediaSinkImpl->linkPad (shared_from_this(), pad);

  if (ret) {
    connectedSinks.push_back (std::weak_ptr<MediaSinkImpl> (mediaSinkImpl) );
  } else {
    gst_element_release_request_pad (getGstreamerElement(), pad);
  }

  g_object_unref (pad);

  mutex.unlock();

  if (!ret) {
    throw KurentoException ("Cannot link pads");
  }
}

void
MediaSourceImpl::removeSink (MediaSinkImpl *mediaSink)
{
  std::shared_ptr<MediaSinkImpl> sinkLocked;
  std::vector< std::weak_ptr<MediaSinkImpl> >::iterator it;

  mutex.lock();

  it = connectedSinks.begin();

  while (it != connectedSinks.end() ) {
    try {
      sinkLocked = (*it).lock();
    } catch (const std::bad_weak_ptr &e) {
    }

    if (sinkLocked == NULL || sinkLocked->getId() == mediaSink->getId() ) {
      it = connectedSinks.erase (it);
    } else {
      it++;
    }
  }

  mutex.unlock();
}

void
MediaSourceImpl::disconnect (MediaSinkImpl *mediaSink)
{
  GST_INFO ("disconnect %" G_GUINT64_FORMAT " from %" G_GUINT64_FORMAT,
            this->getId(), mediaSink->getId() );

  mutex.lock();

  mediaSink->unlink (shared_from_this(), NULL);

  mutex.unlock();
}

std::vector < std::shared_ptr<MediaSink> >
MediaSourceImpl:: getConnectedSinks ()
{
  std::vector < std::shared_ptr<MediaSink> > sinks;

  std::shared_ptr<MediaSinkImpl> sinkLocked;
  std::vector< std::weak_ptr<MediaSinkImpl> >::iterator it;

  mutex.lock();

  for ( it = connectedSinks.begin() ; it != connectedSinks.end(); ++it) {
    try {
      sinkLocked = (*it).lock();
    } catch (const std::bad_weak_ptr &e) {
    }

    if (sinkLocked != NULL) {
      sinks.push_back (std::dynamic_pointer_cast<MediaSink> (sinkLocked) );
    }
  }

  mutex.unlock();

  return sinks;
}

MediaSourceImpl::StaticConstructor MediaSourceImpl::staticConstructor;

MediaSourceImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */

