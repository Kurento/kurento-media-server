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

#ifndef __MEDIA_SOURCE_IMPL_HPP__
#define __MEDIA_SOURCE_IMPL_HPP__

#include "MediaPadImpl.hpp"
#include <generated/MediaSource.hpp>

#include <glibmm.h>

namespace kurento
{

class MediaSinkImpl;

class MediaSourceImpl : public virtual MediaSource, public MediaPadImpl,
  public std::enable_shared_from_this<MediaSourceImpl>
{
public:
  MediaSourceImpl (std::shared_ptr<MediaType> mediaType,
                   const std::string &mediaDescription,
                   std::shared_ptr<MediaObjectImpl> parent);
  virtual ~MediaSourceImpl();

  virtual std::vector<std::shared_ptr<MediaSink>> getConnectedSinks ();
  virtual void connect (std::shared_ptr<MediaSink> sink);

private:
  std::vector < std::weak_ptr<MediaSinkImpl> > connectedSinks;
  void removeSink (MediaSinkImpl *mediaSink);
  void disconnect (MediaSinkImpl *mediaSink);

  Glib::RecMutex mutex;

  const gchar *getPadName ();

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend class MediaSinkImpl;
  friend gboolean link_media_elements (std::shared_ptr<MediaSourceImpl> src,
                                       std::shared_ptr<MediaSinkImpl> sink);
};
} /* kurento */

#endif /* __MEDIA_SOURCE_IMPL_HPP__ */
