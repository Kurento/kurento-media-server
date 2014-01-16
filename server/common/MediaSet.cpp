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

#include "KmsMediaServer_constants.h"
#include "KmsMediaErrorCodes_constants.h"
#include "utils/utils.hpp"

#define GST_CAT_DEFAULT kurento_media_set
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSet"

#define RELEASE_TIMEOUT 10

using namespace kurento;

struct _AutoReleaseData {
  guint timeoutId;
  MediaSet *mediaSet;
  KmsMediaObjectId objectId;
  bool forceRemoving;
};

static gboolean
auto_release (gpointer dataPointer)
{
  AutoReleaseData *data = (AutoReleaseData *) dataPointer;

  GST_TRACE ("Auto release media object %" G_GINT64_FORMAT ", force: %d",
             data->objectId, data->forceRemoving);
  data->mediaSet->unreg (data->objectId, data->forceRemoving);

  return G_SOURCE_CONTINUE;
}

namespace kurento
{

class ObjectReleasing
{
public:
  ~ObjectReleasing () {
    Glib::RefPtr<Glib::TimeoutSource> timeout;
    gint wait_times = 1;

    while (object && !object.unique() ) {
      GST_DEBUG ("Object reference %" G_GINT64_FORMAT " is not unique, "
                 "waiting %d ms", object->id, wait_times * 50);
      g_usleep (wait_times++ * 50000);
    }

    GST_DEBUG ("Destroying object %" G_GINT64_FORMAT, object->id);

    object.reset();
  }

  std::shared_ptr <MediaObjectImpl> object;
};

MediaSet::~MediaSet ()
{
  threadPool.shutdown (true);
}

bool
MediaSet::canBeAutoreleased (const KmsMediaObjectRef &mediaObject)
{
  try {
    getMediaObject<MediaPad> (mediaObject);
    return false;
  } catch (const KmsMediaServerException &e) {
    return true;
  }
}

void
MediaSet::keepAlive (const KmsMediaObjectRef &mediaObject)
throw (KmsMediaServerException)
{
  std::shared_ptr<MediaObjectImpl> mo;
  std::shared_ptr<AutoReleaseData> data;
  std::map<KmsMediaObjectId, std::shared_ptr<AutoReleaseData>>::iterator it;

  if (!canBeAutoreleased (mediaObject) ) {
    GST_DEBUG ("MediaObject %" G_GINT64_FORMAT " is not auto releasable",
               mediaObject.id);
    return;
  }

  /* Check that object exists and it is not exluded from GC */
  mo = getMediaObject<MediaObjectImpl> (mediaObject);

  if (mo->getExcludeFromGC () ) {
    GST_DEBUG ("MediaObject %" G_GINT64_FORMAT " is excluded from GC",
               mediaObject.id);
    return;
  }

  mutex.lock();
  it = mediaObjectsAlive.find (mediaObject.id);

  if (it != mediaObjectsAlive.end() ) {
    data = it->second;

    if (data->timeoutId != 0) {
      g_source_remove (data->timeoutId);
    }
  }

  data->timeoutId = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT,
                    mo->getGarbageCollectorPeriod() * 2, auto_release,
                    (gpointer) data.get (), NULL);
  mutex.unlock();
}

static bool
isForceRemoving (std::shared_ptr<MediaObjectImpl> mediaObject)
{
  return ! std::dynamic_pointer_cast<MediaPipeline> (mediaObject);
}

void
MediaSet::reg (std::shared_ptr<MediaObjectImpl> mediaObject)
{
  std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> >::iterator
      it;
  std::shared_ptr<std::set<KmsMediaObjectId>> children;

  mutex.lock();

  auto findIt = mediaObjectsMap.find (mediaObject->id);

  if (findIt != mediaObjectsMap.end() && findIt->second != NULL) {
    // The object is already in the mediaset
    mutex.unlock();
    return;
  }

  if (mediaObject->parent != NULL) {
    it = childrenMap.find (mediaObject->parent->id);

    if (it != childrenMap.end() ) {
      children = it->second;
    } else {
      children = std::shared_ptr<std::set<KmsMediaObjectId>> (new
                 std::set<KmsMediaObjectId>() );
      childrenMap[mediaObject->parent->id] = children;
    }

    children->insert (mediaObject->id);
  }

  mediaObjectsMap[mediaObject->id] = mediaObject;

  if (!mediaObject->getExcludeFromGC () ) {
    std::shared_ptr<AutoReleaseData> data;

    data = std::shared_ptr<AutoReleaseData> (new AutoReleaseData() );
    data->mediaSet = this;
    data->objectId = mediaObject->id;
    data->forceRemoving = isForceRemoving (mediaObject);

    mediaObjectsAlive[mediaObject->id] = data;
    keepAlive (*mediaObject);
  }

  mutex.unlock();
}

void
MediaSet::removeAutoRelease (const KmsMediaObjectId &id)
{
  std::shared_ptr<AutoReleaseData> data;
  std::map<KmsMediaObjectId, std::shared_ptr<AutoReleaseData>>::iterator it;

  mutex.lock();
  it = mediaObjectsAlive.find (id);

  if (it != mediaObjectsAlive.end() ) {
    data = it->second;

    if (data->timeoutId != 0) {
      g_source_remove (data->timeoutId);
    }
  }

  mediaObjectsAlive.erase (id);
  mutex.unlock();
}

void
MediaSet::unreg (const KmsMediaObjectRef &mediaObject, bool force)
{
  unreg (mediaObject.id, force);
}

void
MediaSet::unreg (const KmsMediaObjectId &id, bool force)
{
  std::map<KmsMediaObjectId, std::shared_ptr<MediaObjectImpl> >::iterator
  mediaObjectsMapIt;
  std::shared_ptr<MediaObjectImpl> mo = NULL;
  std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> >::iterator
      childrenMapIt;
  std::shared_ptr<std::set<KmsMediaObjectId>> children;
  std::set<KmsMediaObjectId>::iterator childrenIt;

  mutex.lock();

  childrenMapIt = childrenMap.find (id);

  if (childrenMapIt != childrenMap.end() ) {
    children = std::shared_ptr<std::set<KmsMediaObjectId>> (new
               std::set<KmsMediaObjectId> (* (childrenMapIt->second) ) );

    if (!force && !children->empty () ) {
      GST_DEBUG ("Media Object %" G_GINT64_FORMAT
                 " has children and not is forcing, so it will not be removed.", id);
      mutex.unlock();
      return;
    }

    for (childrenIt = children->begin(); childrenIt != children->end();
         childrenIt++) {
      unreg (*childrenIt, force);
    }

    childrenMap.erase (id);
  }

  mediaObjectsMapIt = mediaObjectsMap.find (id);

  if (mediaObjectsMapIt != mediaObjectsMap.end() ) {
    mo = mediaObjectsMapIt->second;
  }

  if (mo != NULL) {
    if (mo->parent != NULL) {
      childrenMapIt = childrenMap.find (mo->parent->id);

      if (childrenMapIt != childrenMap.end() ) {
        children = childrenMapIt->second;
        children->erase (mo->id);
      }
    }
  }

  mediaObjectsMap.erase (id);
  removeAutoRelease (id);
  mutex.unlock();

  if (mo) {
    ObjectReleasing *obj = new ObjectReleasing();

    obj->object = mo;
    mo.reset();

    threadPool.push ([obj] () {
      delete obj;
    } );
  }
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
MediaSet::getMediaObject<MediaObjectImpl> (const KmsMediaObjectRef
    &mediaObject);

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
throw (KmsMediaServerException)
{
  std::map<KmsMediaObjectId, std::shared_ptr<MediaObjectImpl> >::iterator it;
  std::shared_ptr<KmsMediaObjectRef> mo = NULL;
  std::shared_ptr<T> typedMo;

  mutex.lock();
  it = mediaObjectsMap.find (mediaObject.id);

  if (it != mediaObjectsMap.end() ) {
    mo = it->second;
  }

  mutex.unlock();

  if (mo == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND,
                                   "Media object not found");
    throw except;
  }

  typedMo = std::dynamic_pointer_cast<T> (mo);

  if (typedMo == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_CAST_ERROR,
                                   "Media Object found is not of requested type");
    throw except;
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
