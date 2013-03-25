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
MediaSet::put (std::shared_ptr<MediaObject> mediaObject)
{
  mediaObjectMap.put (mediaObject->id, mediaObject );
}

void
MediaSet::remove (const MediaObject &mediaObject)
{
  mediaObjectMap.remove (mediaObject.id);
}

int
MediaSet::size ()
{
  return mediaObjectMap.size();
}

std::shared_ptr<MediaFactory>
MediaSet::getMediaFactory (const MediaObject &mediaObject)
{
  std::shared_ptr<MediaObject> mo;
  std::shared_ptr<MediaFactory> mf;

  mo = mediaObjectMap.getValue (mediaObject.id);
  mf = std::dynamic_pointer_cast< MediaFactory > (mo);

  if (mf == NULL )
    throw MediaObjectNotFoundException();

  return mf;
}

std::shared_ptr<Joinable>
MediaSet::getJoinable (const MediaObject &mediaObject)
{
  std::shared_ptr<MediaObject> mo;
  std::shared_ptr<Joinable> j;

  mo = mediaObjectMap.getValue (mediaObject.id);
  j = std::dynamic_pointer_cast< Joinable > (mo);

  if (j == NULL )
    throw MediaObjectNotFoundException();

  return j;
}

std::shared_ptr<MediaPlayer>
MediaSet::getMediaPlayer (const MediaObject &mediaObject)
{
  std::shared_ptr<MediaObject> mo;
  std::shared_ptr< MediaPlayer> mp;

  mo = mediaObjectMap.getValue (mediaObject.id);
  mp = std::dynamic_pointer_cast< MediaPlayer > (mo);

  if (mp == NULL)
    throw MediaObjectNotFoundException();

  return mp;
}

std::shared_ptr<MediaRecorder>
MediaSet::getMediaRecorder (const MediaObject &mediaObject)
{
  std::shared_ptr<MediaObject> mo;
  std::shared_ptr<MediaRecorder> mr;

  mo = mediaObjectMap.getValue (mediaObject.id);
  mr = std::dynamic_pointer_cast<MediaRecorder> (mo);

  if (mr == NULL)
    throw MediaObjectNotFoundException();

  return mr;
}

std::shared_ptr<Stream>
MediaSet::getStream (const MediaObject &mediaObject)
{
  std::shared_ptr<MediaObject> mo;
  std::shared_ptr<Stream> s;

  mo = mediaObjectMap.getValue (mediaObject.id);
  s = std::dynamic_pointer_cast<Stream> (mo);

  if (s == NULL)
    throw MediaObjectNotFoundException();

  return s;
}

std::shared_ptr<Mixer>
MediaSet::getMixer (const MediaObject &mediaObject)
{
  std::shared_ptr<MediaObject> mo;
  std::shared_ptr<Mixer> m;

  mo = mediaObjectMap.getValue (mediaObject.id);
  m = std::dynamic_pointer_cast<Mixer> (mo);

  if (m == NULL)
    throw MediaObjectNotFoundException();

  return m;
}

std::shared_ptr<MixerPort>
MediaSet::getMixerPort (const MediaObject &mediaObject)
{
  std::shared_ptr<MediaObject> mo;
  std::shared_ptr<MixerPort> mp;

  mo = mediaObjectMap.getValue (mediaObject.id);
  mp = std::dynamic_pointer_cast<MixerPort> (mo);

  if (mp == NULL)
    throw MediaObjectNotFoundException();

  return mp;
}

} // kurento

