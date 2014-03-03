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

#ifndef __MEDIA_SINK_IMPL_HPP__
#define __MEDIA_SINK_IMPL_HPP__

#include "MediaPadImpl.hpp"
#include <generated/MediaSink.hpp>

#include <glibmm.h>

namespace kurento
{

class MediaSourceImpl;

class MediaSinkImpl : public virtual MediaSink, public MediaPadImpl
{
public:
  MediaSinkImpl (std::shared_ptr<MediaType> mediaType,
                 const std::string &mediaDescription,
                 std::shared_ptr<MediaObjectImpl> parent);
  virtual ~MediaSinkImpl();

  virtual void disconnect (std::shared_ptr<MediaSource> src);
  virtual std::shared_ptr<MediaSource> getConnectedSrc ();

private:
  std::string getPadName ();

  bool linkPad (std::shared_ptr<MediaSourceImpl> mediaSrc, GstPad *pad);
  void unlink (std::shared_ptr<MediaSourceImpl> mediaSrc, GstPad *sink);
  void unlinkUnchecked (GstPad *sink);

  std::weak_ptr <MediaSourceImpl> connectedSrc;

  Glib::RecMutex mutex;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend class MediaSourceImpl;
  friend gboolean link_media_elements (std::shared_ptr<MediaSourceImpl> src,
                                       std::shared_ptr<MediaSinkImpl> sink);
};
} /* kurento */

#endif /* __MEDIA_SINK_IMPL_HPP__ */
