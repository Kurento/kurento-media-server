/*
 * MediaSrc.hpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
  std::set < std::shared_ptr<MediaSink> > connectedSinks;
  void removeSink (std::shared_ptr<MediaSink> mediaSink);

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
