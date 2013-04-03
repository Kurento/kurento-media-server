/*
 * Joinable.hpp - Kurento Media Server
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

#ifndef __JOINABLE_HPP__
#define __JOINABLE_HPP__

#include "MediaObjectImpl.hpp"
#include "MediaFactory.hpp"
#include "MediaSrc.hpp"
#include "MediaSink.hpp"

namespace kurento
{

class Joinable : public MediaObjectImpl, public std::enable_shared_from_this<Joinable>
{
public:
  Joinable (std::shared_ptr<MediaObject> parent);
  virtual ~Joinable() throw () = 0;

  void join (const Joinable &to);
  void unjoin (const Joinable &to);

  std::vector < std::shared_ptr<MediaSrc> > * getMediaSrcs ();
  std::vector < std::shared_ptr<MediaSink> > * getMediaSinks();
  std::vector < std::shared_ptr<MediaSrc> > * getMediaSrcsByMediaType (const MediaType::type mediaType);
  std::vector < std::shared_ptr<MediaSink> > * getMediaSinksByMediaType (const MediaType::type mediaType);

private:
  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __JOINABLE_HPP__ */
