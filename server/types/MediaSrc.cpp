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
#include "KmsMediaErrorCodes_constants.h"
#include "utils/utils.hpp"

#define GST_CAT_DEFAULT kurento_media_src
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSrc"

#define KEY_DATA_PROPERTY "kms-test-utils-prop"

namespace kurento
{


struct tmp_data {
  std::weak_ptr<MediaSrc> src;
  std::weak_ptr<MediaSink> sink;
  gulong handler;
};

static void
destroy_tmp_data (gpointer data, GClosure *closure)
{
  struct tmp_data *tmp = (struct tmp_data *) data;

  g_slice_free (struct tmp_data, tmp);
}

static struct tmp_data *
create_tmp_data (std::shared_ptr<MediaSrc> src, std::shared_ptr<MediaSink> sink)
{
  struct tmp_data *tmp;

  tmp = g_slice_new0 (struct tmp_data);

  tmp->src = std::weak_ptr<MediaSrc> (src);
  tmp->sink = std::weak_ptr<MediaSink> (sink);
  tmp->handler = 0L;

  return tmp;
}

static void
pad_unlinked (GstPad *pad, GstPad *peer, GstElement *parent)
{
  gst_element_release_request_pad (parent, pad);
}

gboolean
link_media_elements (std::shared_ptr<MediaSrc> src, std::shared_ptr<MediaSink> sink)
{
  bool ret = FALSE;
  GstPad *pad;

  src->mutex.lock ();
  pad = gst_element_get_request_pad (src->getElement(), src->getPadName() );

  if (pad == NULL)
    goto end;

  GST_WARNING ("Connecting pad %s", src->getPadName() );

  g_signal_connect (G_OBJECT (pad), "unlinked", G_CALLBACK (pad_unlinked),
                    src->getElement() );

  if (sink->linkPad (src, pad) ) {
    src->connectedSinks.push_back (std::weak_ptr<MediaSink> (sink) );
    ret = TRUE;
  } else {
    gst_element_release_request_pad (src->getElement(), pad);
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
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  try {
    src = tmp->src.lock();
    sink = tmp->sink.lock();

    if (src && sink && link_media_elements (src, sink) )
      g_signal_handler_disconnect (element, tmp->handler);
  } catch (const std::bad_weak_ptr &e) {
    GST_WARNING ("Removed before connecting");
    g_signal_handler_disconnect (element, tmp->handler);
  }
}

MediaSrc::MediaSrc (std::shared_ptr< kurento::MediaElement > parent, kurento::KmsMediaType::type mediaType)
  : MediaPad (parent, KmsMediaPadDirection::SRC, mediaType)
{

}

MediaSrc::~MediaSrc() throw ()
{
  mutex.lock();

  for (auto it = connectedSinks.begin(); it != connectedSinks.end(); it++) {
    try {
      std::shared_ptr<MediaSink> connectedSinkLocked;

      GST_INFO ("connectedSink");
      connectedSinkLocked = it->lock();

      if (connectedSinkLocked != NULL) {
        connectedSinkLocked->unlinkUnchecked (NULL);
      }
    } catch (const std::bad_weak_ptr &e) {
      GST_WARNING ("Got invalid reference while releasing MediaSrc %"
                   G_GINT64_FORMAT, id);
    }
  }

  mutex.unlock();
}

const gchar *
MediaSrc::getPadName ()
{
  if (mediaType == KmsMediaType::type::AUDIO)
    return (const gchar *) "audio_src_%u";
  else
    return (const gchar *) "video_src_%u";
}

void
MediaSrc::connect (std::shared_ptr<MediaSink> mediaSink) throw (KmsMediaServerException)
{
  GstPad *pad;
  bool ret;

  GST_INFO ("connect %" G_GINT64_FORMAT " to %" G_GINT64_FORMAT, this->id, mediaSink->id);

  mutex.lock();

  pad = gst_element_get_request_pad (getElement(), getPadName() );

  if (pad == NULL) {
    struct tmp_data *tmp;

    GST_DEBUG ("Put connection off until agnostic bin is created for pad %s",
               getPadName() );
    tmp = create_tmp_data (shared_from_this(), mediaSink);
    tmp->handler = g_signal_connect_data (getElement(), "agnosticbin-added",
                                          G_CALLBACK (agnosticbin_added_cb), tmp, destroy_tmp_data,
                                          (GConnectFlags) 0);

    mutex.unlock();
    return;
  }

  g_signal_connect (G_OBJECT (pad), "unlinked", G_CALLBACK (pad_unlinked), getElement() );

  ret = mediaSink->linkPad (shared_from_this(), pad);

  if (ret) {
    connectedSinks.push_back (std::weak_ptr<MediaSink> (mediaSink) );
  } else {
    gst_element_release_request_pad (getElement(), pad);
  }

  g_object_unref (pad);

  mutex.unlock();

  if (!ret) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.CONNECT_ERROR,
                                   "MediaSrc " + std::to_string (this->id) +
                                   " and MediaSink " +
                                   std::to_string (mediaSink->id) +
                                   " cannot be linked");

    throw except;
  }
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
    } catch (const std::bad_weak_ptr &e) {
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

void
MediaSrc:: getConnectedSinks (std::vector < std::shared_ptr<MediaSink> > &_return)
{
  std::shared_ptr<MediaSink> sinkLocked;
  std::vector< std::weak_ptr<MediaSink> >::iterator it;

  _return.clear();

  mutex.lock();

  for ( it = connectedSinks.begin() ; it != connectedSinks.end(); ++it) {
    try {
      sinkLocked = (*it).lock();
    } catch (const std::bad_weak_ptr &e) {
    }

    if (sinkLocked != NULL) {
      _return.push_back (sinkLocked);
    }
  }

  mutex.unlock();
}

MediaSrc::StaticConstructor MediaSrc::staticConstructor;

MediaSrc::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
