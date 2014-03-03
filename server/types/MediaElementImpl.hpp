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

#ifndef __MEDIA_ELEMENT_IMPL_HPP__
#define __MEDIA_ELEMENT_IMPL_HPP__

#include "MediaObjectImpl.hpp"
#include "jsoncpp/json/json.h"
#include <memory>
#include <gst/gst.h>
#include <glibmm.h>

#include <generated/MediaElement.hpp>
#include "MediaSourceImpl.hpp"
#include "MediaSinkImpl.hpp"

namespace kurento
{

class MediaElementImpl : public virtual MediaElement, public MediaObjectImpl,
  public std::enable_shared_from_this<MediaElementImpl>
{
public:
  MediaElementImpl (const std::string &factoryName,
                    std::shared_ptr<MediaObjectImpl> parent, int garbagePeriod);
  virtual ~MediaElementImpl() throw ();

  virtual std::vector<std::shared_ptr<MediaSource>> getMediaSrcs ();
  virtual std::vector<std::shared_ptr<MediaSource>> getMediaSrcs (
        std::shared_ptr<MediaType> mediaType, const std::string &description);
  virtual std::vector<std::shared_ptr<MediaSource>> getMediaSrcs (
        std::shared_ptr<MediaType> mediaType);
  virtual std::vector<std::shared_ptr<MediaSink>> getMediaSinks ();
  virtual std::vector<std::shared_ptr<MediaSink>> getMediaSinks (
        std::shared_ptr<MediaType> mediaType);
  virtual std::vector<std::shared_ptr<MediaSink>> getMediaSinks (
        std::shared_ptr<MediaType> mediaType, const std::string &description);
  virtual void connect (std::shared_ptr<MediaElement> sink,
                        std::shared_ptr<MediaType> mediaType,
                        const std::string &mediaDescription);
  virtual void connect (std::shared_ptr<MediaElement> sink,
                        std::shared_ptr<MediaType> mediaType);
  virtual void connect (std::shared_ptr<MediaElement> sink);

  GstElement *getGstreamerElement() {
    return element;
  };

protected:
  GstElement *element;

private:
  std::weak_ptr<MediaSourceImpl> audioMediaSrc;
  std::weak_ptr<MediaSourceImpl> videoMediaSrc;
  std::weak_ptr<MediaSinkImpl> audioMediaSink;
  std::weak_ptr<MediaSinkImpl> videoMediaSink;

  std::shared_ptr<MediaSourceImpl> getOrCreateAudioMediaSrc();
  std::shared_ptr<MediaSourceImpl> getOrCreateVideoMediaSrc();
  std::shared_ptr<MediaSinkImpl> getOrCreateAudioMediaSink();
  std::shared_ptr<MediaSinkImpl> getOrCreateVideoMediaSink();

  Glib::RecMutex mutex;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __MEDIA_ELEMENT_IMPL_HPP__ */
