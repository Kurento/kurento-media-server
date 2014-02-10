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

#ifndef __MEDIA_ELEMENT_HPP__
#define __MEDIA_ELEMENT_HPP__

#include "MediaPipeline.hpp"
#include "MediaSrc.hpp"
#include "MediaSink.hpp"

#include <glibmm.h>

namespace kurento
{

class MediaElement : public MediaObjectParent,
  public KmsMediaElement,
  public std::enable_shared_from_this<MediaElement>
{
public:
  MediaElement (MediaSet &mediaSet, std::shared_ptr<MediaObjectImpl> parent,
                const std::string &elementType,
                const std::map<std::string, KmsMediaParam> &params,
                const std::string &factoryName);
  virtual ~MediaElement() throw () = 0;

  void getMediaSrcs (std::vector < std::shared_ptr<MediaSrc> > &_return);
  void getMediaSinks (std::vector < std::shared_ptr<MediaSink> > &_return);
  void getMediaSrcsByMediaType (std::vector < std::shared_ptr<MediaSrc> >
                                &_return, const KmsMediaType::type mediaType);
  void getMediaSinksByMediaType (std::vector < std::shared_ptr<MediaSink> >
                                 &_return, const KmsMediaType::type mediaType);

  void connect (std::shared_ptr<MediaElement> sink) throw (
    KmsMediaServerException);
  void connect (std::shared_ptr<MediaElement> sink,
                const KmsMediaType::type mediaType) throw (KmsMediaServerException);

  std::shared_ptr<MediaPipeline> getPipeline () {
    return parent->getPipeline();
  }

protected:
  GstElement *element;

private:
  std::weak_ptr<MediaSrc> audioMediaSrc;
  std::weak_ptr<MediaSrc> videoMediaSrc;
  std::weak_ptr<MediaSink> audioMediaSink;
  std::weak_ptr<MediaSink> videoMediaSink;

  std::shared_ptr<MediaSrc> getOrCreateAudioMediaSrc();
  std::shared_ptr<MediaSrc> getOrCreateVideoMediaSrc();
  std::shared_ptr<MediaSink> getOrCreateAudioMediaSink();
  std::shared_ptr<MediaSink> getOrCreateVideoMediaSink();

  Glib::RecMutex mutex;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend class MediaPad;
};

} // kurento

#endif /* __MEDIA_ELEMENT_HPP__ */
