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

#include "MediaElement.hpp"
#include "KmsMediaErrorCodes_constants.h"
#include "utils/utils.hpp"

#define GST_CAT_DEFAULT kurento_media_element
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaElement"

namespace kurento
{

MediaElement::MediaElement (MediaSet &mediaSet,
                            std::shared_ptr<MediaObjectImpl> parent,
                            const std::string &elementType,
                            const std::map<std::string, KmsMediaParam> &params,
                            const std::string &factoryName)
  : MediaObjectParent (mediaSet, parent, params),
    KmsMediaElement()
{
  element = gst_element_factory_make (factoryName.c_str(), NULL);

  this->elementType = elementType;
  this->objectType.__set_element (*this);
}

MediaElement::~MediaElement () throw ()
{
}

std::shared_ptr<MediaSrc>
MediaElement::getOrCreateAudioMediaSrc()
{
  mutex.lock();

  std::shared_ptr<MediaSrc> locked;

  try {
    locked = audioMediaSrc.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSrc> (new  MediaSrc (shared_from_this(),
                                        KmsMediaType::type::AUDIO) );
    audioMediaSrc = std::weak_ptr<MediaSrc> (locked);
    registerChild (locked);
  }

  mutex.unlock();

  return locked;
}

std::shared_ptr<MediaSrc>
MediaElement::getOrCreateVideoMediaSrc()
{
  mutex.lock();

  std::shared_ptr<MediaSrc> locked;

  try {
    locked = videoMediaSrc.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSrc> (new  MediaSrc (shared_from_this(),
                                        KmsMediaType::type::VIDEO) );
    videoMediaSrc = std::weak_ptr<MediaSrc> (locked);
    registerChild (locked);
  }

  mutex.unlock();

  return locked;
}

std::shared_ptr<MediaSink>
MediaElement::getOrCreateAudioMediaSink()
{
  mutex.lock();

  std::shared_ptr<MediaSink> locked;

  try {
    locked = audioMediaSink.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSink> (new  MediaSink (shared_from_this(),
                                         KmsMediaType::type::AUDIO) );
    audioMediaSink = std::weak_ptr<MediaSink> (locked);
    registerChild (locked);
  }

  mutex.unlock();

  return locked;
}

std::shared_ptr<MediaSink>
MediaElement::getOrCreateVideoMediaSink()
{
  mutex.lock();

  std::shared_ptr<MediaSink> locked;

  try {
    locked = videoMediaSink.lock();
  } catch (const std::bad_weak_ptr &e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSink> (new  MediaSink (shared_from_this(),
                                         KmsMediaType::type::VIDEO) );
    videoMediaSink = std::weak_ptr<MediaSink> (locked);
    registerChild (locked);
  }

  mutex.unlock();

  return locked;
}

void
MediaElement::getMediaSrcs (std::vector < std::shared_ptr<MediaSrc> > &_return)
{
  _return.push_back (getOrCreateAudioMediaSrc() );
  _return.push_back (getOrCreateVideoMediaSrc() );
}

void
MediaElement::getMediaSinks (std::vector < std::shared_ptr<MediaSink> >
                             &_return)
{
  _return.push_back (getOrCreateAudioMediaSink() );
  _return.push_back (getOrCreateVideoMediaSink() );
}

void
MediaElement::getMediaSrcsByMediaType (
  std::vector < std::shared_ptr<MediaSrc> > &_return,
  const KmsMediaType::type mediaType)
{
  if (mediaType == KmsMediaType::type::AUDIO) {
    _return.push_back (getOrCreateAudioMediaSrc() );
  } else if (mediaType == KmsMediaType::type::VIDEO) {
    _return.push_back (getOrCreateVideoMediaSrc() );
  }
}

void
MediaElement::getMediaSinksByMediaType (
  std::vector < std::shared_ptr<MediaSink> > &_return,
  const KmsMediaType::type mediaType)
{
  if (mediaType == KmsMediaType::type::AUDIO) {
    _return.push_back (getOrCreateAudioMediaSink() );
  } else if (mediaType == KmsMediaType::type::VIDEO) {
    _return.push_back (getOrCreateVideoMediaSink() );
  }
}

void
MediaElement::connect (std::shared_ptr<MediaElement> sink)
throw (KmsMediaServerException)
{
  std::shared_ptr<MediaSrc> audio_src = getOrCreateAudioMediaSrc();
  std::shared_ptr<MediaSink> audio_sink = sink->getOrCreateAudioMediaSink();

  std::shared_ptr<MediaSrc> video_src = getOrCreateVideoMediaSrc();
  std::shared_ptr<MediaSink> video_sink = sink->getOrCreateVideoMediaSink();

  audio_src->connect (audio_sink);

  try {
    video_src->connect (video_sink);
  } catch (...) {
    try {
      audio_src->disconnect (audio_sink);
    } catch (...) {
    }

    throw;
  }
}

void
MediaElement::connect (std::shared_ptr<MediaElement> sink,
                       const KmsMediaType::type mediaType)
throw (KmsMediaServerException)
{
  std::shared_ptr<MediaSrc> mediaSrc;
  std::shared_ptr<MediaSink> mediaSink;

  if (mediaType == KmsMediaType::AUDIO) {
    mediaSrc = getOrCreateAudioMediaSrc();
    mediaSink = sink->getOrCreateAudioMediaSink();
  } else if (mediaType == KmsMediaType::VIDEO) {
    mediaSrc = getOrCreateVideoMediaSrc();
    mediaSink = sink->getOrCreateVideoMediaSink();
  } else {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.UNSUPPORTED_MEDIA_TYPE,
                                   "Unsupported media type");
    throw except;
  }

  mediaSrc->connect (mediaSink);
}
MediaElement::StaticConstructor MediaElement::staticConstructor;

MediaElement::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
