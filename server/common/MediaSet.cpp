/*
 * MediaSet.cpp - Kurento Media Server
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

#include "MediaSet.hpp"

namespace kurento
{

void
MediaSet::put (MediaObject *mediaObject)
{
  mediaObjectMap.put (mediaObject->id, mediaObject);
}

int
MediaSet::size ()
{
  return mediaObjectMap.size();
}

MediaFactory *
MediaSet::getMediaFactory (const MediaObject &mediaObject)
{
  MediaObject *mo;
  MediaFactory *mf;

  mo = mediaObjectMap.getValue (mediaObject.id);
  mf = dynamic_cast<MediaFactory *> (mo);

  if (mf == NULL )
    throw MediaObjectNotFoundException();

  return mf;
}

Joinable *
MediaSet::getJoinable (const MediaObject &mediaObject)
{
  MediaObject *mo;
  Joinable *j;

  mo = mediaObjectMap.getValue (mediaObject.id);
  j = dynamic_cast<Joinable *> (mo);

  if (j == NULL )
    throw MediaObjectNotFoundException();

  return j;
}

MediaPlayer *
MediaSet::getMediaPlayer (const MediaObject &mediaObject)
{
  MediaObject *mo;
  MediaPlayer *mp;

  mo = mediaObjectMap.getValue (mediaObject.id);
  mp = dynamic_cast<MediaPlayer *> (mo);

  if (mp == NULL)
    throw MediaObjectNotFoundException();

  return mp;
}

MediaRecorder *
MediaSet::getMediaRecorder (const MediaObject &mediaObject)
{
  MediaObject *mo;
  MediaRecorder *mr;

  mo = mediaObjectMap.getValue (mediaObject.id);
  mr = dynamic_cast<MediaRecorder *> (mo);

  if (mr == NULL)
    throw MediaObjectNotFoundException();

  return mr;
}

} // kurento

