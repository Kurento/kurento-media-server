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
#include "types/MediaManager.hpp"
#include "types/MediaElement.hpp"
#include "types/MediaSrc.hpp"
#include "types/Mixer.hpp"
#include "types/UriEndPoint.hpp"
#include "types/HttpEndPoint.hpp"
#include "types/SdpEndPoint.hpp"
#include "types/Filter.hpp"

namespace kurento
{

void
MediaSet::put (std::shared_ptr<MediaObjectImpl> mediaObject)
{
  std::map<ObjectId, std::shared_ptr<std::set<ObjectId>> >::iterator it;
  std::shared_ptr<std::set<ObjectId>> children;

  mutex.lock();

  if (mediaObject->parent != NULL) {
    it = childrenMap.find (mediaObject->parent->id);

    if (it != childrenMap.end() ) {
      children = it->second;
    } else {
      children = std::shared_ptr<std::set<ObjectId>> (new std::set<ObjectId>() );
      childrenMap[mediaObject->parent->id] = children;
    }

    children->insert (mediaObject->id);
  }

  mediaObjectsMap[mediaObject->id] = mediaObject;
  mutex.unlock();
}

void
MediaSet::remove (const MediaObjectId &mediaObject)
{
  remove (mediaObject.id);
}

void
MediaSet::remove (const ObjectId &id)
{
  std::map<ObjectId, std::shared_ptr<MediaObjectImpl> >::iterator mediaObjectsMapIt;
  std::shared_ptr<MediaObjectImpl> mo = NULL;
  std::map<ObjectId, std::shared_ptr<std::set<ObjectId>> >::iterator childrenMapIt;
  std::shared_ptr<std::set<ObjectId>> children;
  std::set<ObjectId>::iterator childrenIt;

  mutex.lock();

  mediaObjectsMapIt = mediaObjectsMap.find (id);

  if (mediaObjectsMapIt != mediaObjectsMap.end() )
    mo = mediaObjectsMapIt->second;

  if (mo != NULL) {
    if (mo->parent != NULL) {
      childrenMapIt = childrenMap.find (mo->parent->id);

      if (childrenMapIt != childrenMap.end() ) {
        children = childrenMapIt->second;
        children->erase (mo->id);
      }
    }
  }

  childrenMapIt = childrenMap.find (id);

  if (childrenMapIt != childrenMap.end() ) {
    children = std::shared_ptr<std::set<ObjectId>> (new std::set<ObjectId> (* (childrenMapIt->second) ) );

    for (childrenIt = children->begin(); childrenIt != children->end(); childrenIt++) {
      remove (*childrenIt);
    }

    childrenMap.erase (id);
  }

  mediaObjectsMap.erase (id);
  mutex.unlock();
}

int
MediaSet::size ()
{
  int size;

  mutex.lock();
  size = mediaObjectsMap.size();
  mutex.unlock();

  return size;
}

template std::shared_ptr<MediaObjectImpl>
MediaSet::getMediaObject<MediaObjectImpl> (const MediaObjectId &mediaObject);

template std::shared_ptr<MediaManager>
MediaSet::getMediaObject<MediaManager> (const MediaObjectId &mediaObject);

template std::shared_ptr<MediaElement>
MediaSet::getMediaObject<MediaElement> (const MediaObjectId &mediaObject);

template std::shared_ptr<MediaPad>
MediaSet::getMediaObject<MediaPad> (const MediaObjectId &mediaObject);

template std::shared_ptr<MediaSrc>
MediaSet::getMediaObject<MediaSrc> (const MediaObjectId &mediaObject);

template std::shared_ptr<MediaSink>
MediaSet::getMediaObject<MediaSink> (const MediaObjectId &mediaObject);

template std::shared_ptr<Mixer>
MediaSet::getMediaObject<Mixer> (const MediaObjectId &mediaObject);

template std::shared_ptr<UriEndPoint>
MediaSet::getMediaObject<UriEndPoint> (const MediaObjectId &mediaObject);

template std::shared_ptr<HttpEndPoint>
MediaSet::getMediaObject<HttpEndPoint> (const MediaObjectId &mediaObject);

template std::shared_ptr<SdpEndPoint>
MediaSet::getMediaObject<SdpEndPoint> (const MediaObjectId &mediaObject);

template std::shared_ptr<Filter>
MediaSet::getMediaObject<Filter> (const MediaObjectId &mediaObject);

template <class T> std::shared_ptr<T>
MediaSet::getMediaObject (const MediaObjectId &mediaObject)
{
  std::map<ObjectId, std::shared_ptr<MediaObjectImpl> >::iterator it;
  std::shared_ptr<MediaObjectId> mo = NULL;
  std::shared_ptr<T> typedMo;

  mutex.lock();
  it = mediaObjectsMap.find (mediaObject.id);

  if (it != mediaObjectsMap.end() )
    mo = it->second;

  mutex.unlock();

  if (mo == NULL)
    throw MediaObjectNotFoundException();

  typedMo = std::dynamic_pointer_cast<T> (mo);

  if (typedMo == NULL)
    throw MediaObjectNotFoundException();

  return typedMo;
}

} // kurento
