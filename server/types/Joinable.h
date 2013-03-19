/*
 * Joinable.h - Kurento Media Server
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

#ifndef JOINABLE_H
#define JOINABLE_H

#include "types/MediaObjectImpl.h"
#include "types/MediaFactory.h"
#include "types/MediaSrc.h"
#include "types/MediaSink.h"

namespace kurento
{

class Joinable : public virtual MediaObjectImpl
{
public:
  Joinable (MediaFactory &mediaFactory);
  virtual ~Joinable() throw () = 0;

  void join (const Joinable &to);
  void unjoin (Joinable &to);

  void getMediaSrcs (std::vector < MediaSrc > &_return);
  void getMediaSinks (std::vector < MediaSink > &_return);
  void getMediaSrcsByMediaType (std::vector < MediaSrc > &_return, const MediaType::type mediaType);
  void getMediaSinksByMediaType (std::vector < MediaSink > &_return, const MediaType::type mediaType);
};

} // kurento

#endif /* JOINABLE_H */
