/*
 * MediaElement.hpp - Kurento Media Server
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

#ifndef __MEDIA_ELEMENT_HPP__
#define __MEDIA_ELEMENT_HPP__

#include "MediaManager.hpp"
#include "MediaSrc.hpp"
#include "MediaSink.hpp"

#include <glibmm.h>

namespace kurento
{

class MediaElement : public MediaObjectImpl, public std::enable_shared_from_this<MediaElement>
{
public:
  MediaElement (std::shared_ptr<MediaObjectImpl> parent);
  virtual ~MediaElement() throw () = 0;

  std::vector < std::shared_ptr<MediaSrc> > * getMediaSrcs ();
  std::vector < std::shared_ptr<MediaSink> > * getMediaSinks();
  std::vector < std::shared_ptr<MediaSrc> > * getMediaSrcsByMediaType (const MediaType::type mediaType);
  std::vector < std::shared_ptr<MediaSink> > * getMediaSinksByMediaType (const MediaType::type mediaType);

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
};

} // kurento

#endif /* __MEDIA_ELEMENT_HPP__ */
