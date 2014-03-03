/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#include "MediaElementImpl.hpp"
#include "MediaPipelineImpl.hpp"
#include <common/MediaSet.hpp>

#define GST_CAT_DEFAULT kurento_media_element_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaElementImpl"

namespace kurento
{

MediaElementImpl::MediaElementImpl (const std::string &factoryName,
                                    std::shared_ptr<MediaObjectImpl> parent,
                                    int garbagePeriod) :
  MediaObjectImpl (parent, garbagePeriod)
{
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );

  element = gst_element_factory_make (factoryName.c_str(), NULL);

  g_object_ref (element);
  gst_bin_add (GST_BIN ( pipe->getPipeline() ), element);
  gst_element_sync_state_with_parent (element);
}

MediaElementImpl::~MediaElementImpl()
{
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );

  gst_bin_remove (GST_BIN ( pipe->getPipeline() ), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

std::shared_ptr<MediaSourceImpl>
MediaElementImpl::getOrCreateAudioMediaSrc()
{
  mutex.lock();

  std::shared_ptr<MediaSourceImpl> locked;

  try {
    locked = audioMediaSrc.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    std::shared_ptr<MediaType> mediaType (new MediaType (MediaType::AUDIO) );

    locked = std::shared_ptr<MediaSourceImpl> (
               new  MediaSourceImpl (mediaType, "", shared_from_this() ) );
    audioMediaSrc = std::weak_ptr<MediaSourceImpl> (locked);

    MediaSet::getMediaSet()->reg (locked);
  }

  mutex.unlock();

  return locked;
}

std::shared_ptr<MediaSourceImpl>
MediaElementImpl::getOrCreateVideoMediaSrc()
{
  mutex.lock();

  std::shared_ptr<MediaSourceImpl> locked;

  try {
    locked = videoMediaSrc.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    std::shared_ptr<MediaType> mediaType (new MediaType (MediaType::VIDEO) );

    locked = std::shared_ptr<MediaSourceImpl> (
               new  MediaSourceImpl (mediaType, "", shared_from_this() ) );
    videoMediaSrc = std::weak_ptr<MediaSourceImpl> (locked);

    MediaSet::getMediaSet()->reg (locked);
  }

  mutex.unlock();

  return locked;
}

std::shared_ptr<MediaSinkImpl>
MediaElementImpl::getOrCreateAudioMediaSink()
{
  mutex.lock();

  std::shared_ptr<MediaSinkImpl> locked;

  try {
    locked = audioMediaSink.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    std::shared_ptr<MediaType> mediaType (new MediaType (MediaType::AUDIO) );

    locked = std::shared_ptr<MediaSinkImpl> (
               new  MediaSinkImpl (mediaType, "", shared_from_this() ) );

    audioMediaSink = std::weak_ptr<MediaSinkImpl> (locked);

    MediaSet::getMediaSet()->reg (locked);
  }

  mutex.unlock();

  return locked;
}

std::shared_ptr<MediaSinkImpl>
MediaElementImpl::getOrCreateVideoMediaSink()
{
  mutex.lock();

  std::shared_ptr<MediaSinkImpl> locked;

  try {
    locked = videoMediaSink.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    std::shared_ptr<MediaType> mediaType (new MediaType (MediaType::VIDEO) );

    locked = std::shared_ptr<MediaSinkImpl> (
               new  MediaSinkImpl (mediaType, "", shared_from_this() ) );

    videoMediaSink = std::weak_ptr<MediaSinkImpl> (locked);

    MediaSet::getMediaSet()->reg (locked);
  }

  mutex.unlock();

  return locked;
}

std::vector<std::shared_ptr<MediaSource>>
                                       MediaElementImpl::getMediaSrcs ()
{
  std::vector<std::shared_ptr<MediaSource>> srcs;

  srcs.push_back (getOrCreateAudioMediaSrc() );
  srcs.push_back (getOrCreateVideoMediaSrc() );

  return srcs;
}

std::vector<std::shared_ptr<MediaSink>>
                                     MediaElementImpl::getMediaSinks ()
{
  std::vector<std::shared_ptr<MediaSink>> sinks;

  sinks.push_back (getOrCreateAudioMediaSink() );
  sinks.push_back (getOrCreateVideoMediaSink() );

  return sinks;
}

std::vector<std::shared_ptr<MediaSource>>
                                       MediaElementImpl::getMediaSrcs (
                                           std::shared_ptr<MediaType> mediaType, const std::string &description)
{
  if (description == "")  {
    return getMediaSrcs (mediaType);
  } else {
    std::vector<std::shared_ptr<MediaSource>> srcs;

    return srcs;
  }
}

std::vector<std::shared_ptr<MediaSink>>
                                     MediaElementImpl::getMediaSinks (
                                       std::shared_ptr<MediaType> mediaType, const std::string &description)
{
  if (description == "")  {
    return getMediaSinks (mediaType);
  } else {
    std::vector<std::shared_ptr<MediaSink>> sinks;

    return sinks;
  }
}

std::vector<std::shared_ptr<MediaSource>>
                                       MediaElementImpl::getMediaSrcs (std::shared_ptr<MediaType> mediaType)
{
  std::vector<std::shared_ptr<MediaSource>> srcs;

  if (mediaType->getValue() == MediaType::AUDIO) {
    srcs.push_back (getOrCreateAudioMediaSrc() );
  } else if (mediaType->getValue() == MediaType::VIDEO) {
    srcs.push_back (getOrCreateVideoMediaSrc() );
  }

  return srcs;
}

std::vector<std::shared_ptr<MediaSink>>
                                     MediaElementImpl::getMediaSinks (std::shared_ptr<MediaType> mediaType)
{
  std::vector<std::shared_ptr<MediaSink>> sinks;

  if (mediaType->getValue() == MediaType::AUDIO) {
    sinks.push_back (getOrCreateAudioMediaSink() );
  } else if (mediaType->getValue() == MediaType::VIDEO) {
    sinks.push_back (getOrCreateVideoMediaSink() );
  }

  return sinks;
}

void
MediaElementImpl::connect (std::shared_ptr<MediaElement> sink)
{
  std::shared_ptr<MediaElementImpl> sinkImpl =
    std::dynamic_pointer_cast<MediaElementImpl> (sink);

  std::shared_ptr<MediaSource> audio_src = getOrCreateAudioMediaSrc();
  std::shared_ptr<MediaSink> audio_sink = sinkImpl->getOrCreateAudioMediaSink();

  std::shared_ptr<MediaSource> video_src = getOrCreateVideoMediaSrc();
  std::shared_ptr<MediaSink> video_sink = sinkImpl->getOrCreateVideoMediaSink();

  audio_src->connect (audio_sink);

  try {
    video_src->connect (video_sink);
  } catch (...) {
    try {
      audio_sink->disconnect (audio_src);
    } catch (...) {
    }

    throw;
  }
}

void
MediaElementImpl::connect (std::shared_ptr< MediaElement > sink,
                           std::shared_ptr< MediaType > mediaType,
                           const std::string &mediaDescription)
{
  if (mediaDescription == "") {
    connect (sink, mediaType);
  }
}

void
MediaElementImpl::connect (std::shared_ptr< MediaElement > sink,
                           std::shared_ptr< MediaType > mediaType)
{
  std::shared_ptr<MediaElementImpl> sinkImpl =
    std::dynamic_pointer_cast<MediaElementImpl> (sink);

  if (mediaType->getValue() == MediaType::AUDIO) {
    std::shared_ptr<MediaSource> audio_src = getOrCreateAudioMediaSrc();
    std::shared_ptr<MediaSink> audio_sink = sinkImpl->getOrCreateAudioMediaSink();

    audio_src->connect (audio_sink);
  } else if (mediaType->getValue() == MediaType::VIDEO) {
    std::shared_ptr<MediaSource> video_src = getOrCreateVideoMediaSrc();
    std::shared_ptr<MediaSink> video_sink = sinkImpl->getOrCreateVideoMediaSink();

    video_src->connect (video_sink);
  }
}

MediaElementImpl::StaticConstructor MediaElementImpl::staticConstructor;

MediaElementImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
