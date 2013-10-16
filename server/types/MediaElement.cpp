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

#define GST_CAT_DEFAULT kurento_media_element
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaElement"

namespace kurento
{

MediaElement::MediaElement (std::shared_ptr<MediaObjectImpl> parent, const std::string elementType,
                            const std::map<std::string, KmsMediaParam> &params)
  : MediaObjectImpl (parent, params),
    KmsMediaElement()
{
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
  } catch (std::bad_weak_ptr e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSrc> (new  MediaSrc (shared_from_this(), KmsMediaType::type::AUDIO) );
    audioMediaSrc = std::weak_ptr<MediaSrc> (locked);
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
  } catch (std::bad_weak_ptr e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSrc> (new  MediaSrc (shared_from_this(), KmsMediaType::type::VIDEO) );
    videoMediaSrc = std::weak_ptr<MediaSrc> (locked);
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
  } catch (std::bad_weak_ptr e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSink> (new  MediaSink (shared_from_this(), KmsMediaType::type::AUDIO) );
    audioMediaSink = std::weak_ptr<MediaSink> (locked);
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
  } catch (std::bad_weak_ptr e) {
  }

  if (locked.get() == NULL) {
    locked = std::shared_ptr<MediaSink> (new  MediaSink (shared_from_this(), KmsMediaType::type::VIDEO) );
    videoMediaSink = std::weak_ptr<MediaSink> (locked);
  }

  mutex.unlock();

  return locked;
}

std::vector < std::shared_ptr<MediaSrc> > *
MediaElement::getMediaSrcs ()
{
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs = new std::vector< std::shared_ptr<MediaSrc> >();

  mediaSrcs->push_back (getOrCreateAudioMediaSrc() );
  mediaSrcs->push_back (getOrCreateVideoMediaSrc() );

  return mediaSrcs;
}

std::vector < std::shared_ptr<MediaSink> > *
MediaElement::getMediaSinks()
{
  std::vector< std::shared_ptr<MediaSink> > *mediaSinks = new std::vector< std::shared_ptr<MediaSink> >();

  mediaSinks->push_back (getOrCreateAudioMediaSink() );
  mediaSinks->push_back (getOrCreateVideoMediaSink() );

  return mediaSinks;
}

std::vector < std::shared_ptr<MediaSrc> > *
MediaElement::getMediaSrcsByMediaType (const KmsMediaType::type mediaType)
{
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs = new std::vector< std::shared_ptr<MediaSrc> >();

  if (mediaType == KmsMediaType::type::AUDIO)
    mediaSrcs->push_back (getOrCreateAudioMediaSrc() );
  else if (mediaType == KmsMediaType::type::VIDEO)
    mediaSrcs->push_back (getOrCreateVideoMediaSrc() );

  return mediaSrcs;
}

std::vector < std::shared_ptr<MediaSink> > *
MediaElement::getMediaSinksByMediaType (const KmsMediaType::type mediaType)
{
  std::vector< std::shared_ptr<MediaSink> > *mediaSinks = new std::vector< std::shared_ptr<MediaSink> >();

  if (mediaType == KmsMediaType::type::AUDIO)
    mediaSinks->push_back (getOrCreateAudioMediaSink() );
  else if (mediaType == KmsMediaType::type::VIDEO)
    mediaSinks->push_back (getOrCreateVideoMediaSink() );

  return mediaSinks;
}

MediaElement::StaticConstructor MediaElement::staticConstructor;

MediaElement::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
