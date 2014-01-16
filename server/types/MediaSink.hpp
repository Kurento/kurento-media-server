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

#ifndef __MEDIA_SINK_HPP__
#define __MEDIA_SINK_HPP__

#include "MediaPad.hpp"
#include "MediaSrc.hpp"

#include <glibmm.h>

namespace kurento
{

class MediaElement;

class MediaSink : public MediaPad,
  public std::enable_shared_from_this<MediaSink>
{
public:
  MediaSink (std::shared_ptr<MediaElement> parent, KmsMediaType::type mediaType);
  ~MediaSink() throw ();

  std::shared_ptr<MediaSrc> getConnectedSrc ();

private:
  std::string getPadName ();

  bool linkPad (std::shared_ptr<MediaSrc> mediaSrc, GstPad *pad);
  void unlink (std::shared_ptr<MediaSrc> mediaSrc, GstPad *sink);
  void unlinkUnchecked (GstPad *sink);

  std::weak_ptr <MediaSrc> connectedSrc;

  Glib::RecMutex mutex;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend MediaSrc::~MediaSrc ();
  friend void MediaSrc::disconnect (MediaSink *mediaSink);
  friend void MediaSrc::connect (std::shared_ptr<MediaSink> mediaSink);
  friend gboolean link_media_elements (std::shared_ptr<MediaSrc> src,
                                       std::shared_ptr<MediaSink> sink);
};

} // kurento

#endif /* __MEDIA_SINK_HPP__ */
