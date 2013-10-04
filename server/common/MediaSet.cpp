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

#include "MediaSet.hpp"
#include "types/MediaPipeline.hpp"
#include "types/MediaElement.hpp"
#include "types/MediaSrc.hpp"
#include "types/Mixer.hpp"
#include "types/UriEndPoint.hpp"
#include "types/HttpEndPoint.hpp"
#include "types/SdpEndPoint.hpp"
#include "types/Filter.hpp"

#include "KmsMediaErrorCodes_constants.h"
#include "utils/utils.hpp"

#define GST_CAT_DEFAULT kurento_media_set
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSet"

namespace kurento
{

void
MediaSet::put (std::shared_ptr<MediaObjectImpl> mediaObject)
{
  std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> >::iterator it;
  std::shared_ptr<std::set<KmsMediaObjectId>> children;

  mutex.lock();

  if (mediaObject->parent != NULL) {
    it = childrenMap.find (mediaObject->parent->id);

    if (it != childrenMap.end() ) {
      children = it->second;
    } else {
      children = std::shared_ptr<std::set<KmsMediaObjectId>> (new std::set<KmsMediaObjectId>() );
      childrenMap[mediaObject->parent->id] = children;
    }

    children->insert (mediaObject->id);
  }

  mediaObjectsMap[mediaObject->id] = mediaObject;
  mutex.unlock();
}

void
MediaSet::remove (const KmsMediaObjectRef &mediaObject)
{
  remove (mediaObject.id);
}

void
MediaSet::remove (const KmsMediaObjectId &id)
{
  std::map<KmsMediaObjectId, std::shared_ptr<MediaObjectImpl> >::iterator mediaObjectsMapIt;
  std::shared_ptr<MediaObjectImpl> mo = NULL;
  std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> >::iterator childrenMapIt;
  std::shared_ptr<std::set<KmsMediaObjectId>> children;
  std::set<KmsMediaObjectId>::iterator childrenIt;

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
    children = std::shared_ptr<std::set<KmsMediaObjectId>> (new std::set<KmsMediaObjectId> (* (childrenMapIt->second) ) );

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
MediaSet::getMediaObject<MediaObjectImpl> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<MediaPipeline>
MediaSet::getMediaObject<MediaPipeline> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<MediaElement>
MediaSet::getMediaObject<MediaElement> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<MediaPad>
MediaSet::getMediaObject<MediaPad> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<MediaSrc>
MediaSet::getMediaObject<MediaSrc> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<MediaSink>
MediaSet::getMediaObject<MediaSink> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<Mixer>
MediaSet::getMediaObject<Mixer> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<UriEndPoint>
MediaSet::getMediaObject<UriEndPoint> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<HttpEndPoint>
MediaSet::getMediaObject<HttpEndPoint> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<SdpEndPoint>
MediaSet::getMediaObject<SdpEndPoint> (const KmsMediaObjectRef &mediaObject);

template std::shared_ptr<Filter>
MediaSet::getMediaObject<Filter> (const KmsMediaObjectRef &mediaObject);

template <class T> std::shared_ptr<T>
MediaSet::getMediaObject (const KmsMediaObjectRef &mediaObject)
{
  std::map<KmsMediaObjectId, std::shared_ptr<MediaObjectImpl> >::iterator it;
  std::shared_ptr<KmsMediaObjectRef> mo = NULL;
  std::shared_ptr<T> typedMo;

  mutex.lock();
  it = mediaObjectsMap.find (mediaObject.id);

  if (it != mediaObjectsMap.end() )
    mo = it->second;

  mutex.unlock();

  if (mo == NULL) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND, "Media object not found");
  }

  typedMo = std::dynamic_pointer_cast<T> (mo);

  if (typedMo == NULL) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_CAST_ERROR, "Media Object found is not of requested type");
  }

  return typedMo;
}

MediaSet::StaticConstructor MediaSet::staticConstructor;

MediaSet::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
