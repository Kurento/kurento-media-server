/*
 * MediaSink.h - Kurento Media Server
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

#ifndef MEDIA_SINK_H
#define MEDIA_SINK_H

#include "types/MediaObjectImpl.h"
#include "types/MediaFactory.h"

namespace kurento
{

class MediaSrc;

class MediaSink : public virtual MediaObjectImpl
{
public:
  MediaSink (MediaFactory &mediaFactory);
  ~MediaSink() throw ();

  MediaType::type getMediaType ();
  void getConnectedSrc (const MediaSrc &mediaSrc);
};

} // kurento

#endif /* MEDIA_SINK_H */
