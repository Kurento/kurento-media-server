/*
 * MediaPad.hpp - Kurento Media Server
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

#ifndef __MEDIA_PAD_HPP__
#define __MEDIA_PAD_HPP__

#include "MediaObjectImpl.hpp"

namespace kurento
{

class MediaElement;

class MediaPad : public MediaObjectImpl
{
public:
  MediaPad (std::shared_ptr<MediaElement> parent, MediaType::type, MediaPadType::type);
  virtual ~MediaPad() throw () = 0;

  MediaType::type getMediaType ();

protected:
  GstElement * getElement();

private:
  MediaType::type mediaType;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __MEDIA_PAD_HPP__ */
