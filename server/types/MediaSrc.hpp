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

#ifndef __MEDIA_SRC_HPP__
#define __MEDIA_SRC_HPP__

#include "MediaPad.hpp"
#include <glibmm.h>

namespace kurento
{

class MediaSink;

class MediaSrc : public MediaPad, public std::enable_shared_from_this<MediaSrc>
{
public:
  MediaSrc (std::shared_ptr<MediaElement> parent, MediaType::type mediaType);
  ~MediaSrc() throw ();

  void connect (std::shared_ptr<MediaSink> mediaSink);
  void disconnect (std::shared_ptr<MediaSink> mediaSink);
  std::vector < std::shared_ptr<MediaSink> > * getConnectedSinks ();

private:
  std::vector < std::weak_ptr<MediaSink> > connectedSinks;
  void removeSink (MediaSink *mediaSink);
  void disconnect (MediaSink *mediaSink);

  Glib::RecMutex mutex;

  std::string getPadName ();

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend class MediaSink;
};

} // kurento

#endif /* __MEDIA_SRC_HPP__ */
