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

namespace kurento
{

struct _KeepAliveData {
  guint timeoutId;
  MediaSet *mediaSet;
  KmsMediaObjectId objectId;
};

static gboolean
keep_alive_time_out (gpointer dataPointer)
{
  KeepAliveData *data = (KeepAliveData *) dataPointer;

  GST_TRACE ("Auto unreg MediaObject %" G_GINT64_FORMAT, data->objectId);
  data->mediaSet->unreg (data->objectId, false);

  return G_SOURCE_CONTINUE;
}

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

static void
eraseFromChildren (std::shared_ptr<MediaObjectImpl> mo,
                   std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> >
                   &childrenMap)
{
  if (mo->parent == NULL) {
    return;
  }

  auto it = childrenMap.find (mo->parent->id);

  if (it != childrenMap.end() ) {
    std::shared_ptr<std::set<KmsMediaObjectId>> children;

    children = it->second;
    children->erase (mo->id);
  }
}

static void
insertIntoChildren (std::shared_ptr<MediaObjectImpl> mo,
                    std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> >
                    &childrenMap)
{
  std::shared_ptr<std::set<KmsMediaObjectId>> children;

  if (mo->parent == NULL) {
    return;
  }

  auto it = childrenMap.find (mo->parent->id);

  if (it != childrenMap.end() ) {
    children = it->second;
  } else {
    children = std::shared_ptr<std::set<KmsMediaObjectId>> (new
               std::set<KmsMediaObjectId>() );
    childrenMap[mo->parent->id] = children;
  }

  children->insert (mo->id);
}

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
  std::shared_ptr<KeepAliveData> data;
  std::map<KmsMediaObjectId, std::shared_ptr<KeepAliveData>>::iterator it;

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

    data->timeoutId = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT,
                      mo->getGarbageCollectorPeriod() * 2, keep_alive_time_out,
                      (gpointer) data.get (), NULL);
  }

  mutex.unlock();
}

void
MediaSet::reg (std::shared_ptr<MediaObjectImpl> mediaObject)
{
  std::shared_ptr<MediaObjectImpl> mo;

  mutex.lock();

  mo = mediaObjectToRef (mediaObject->id);

  if (mo != NULL) {
    if (mediaObject.get () != mo.get () ) {
      GST_ERROR ("There is another MediaObject registered "
                 "with the same id (%" G_GINT64_FORMAT ")", mo->id);
    } else {
      GST_DEBUG ("MediaObject %" G_GINT64_FORMAT " is already in the MediaSet",
                 mo->id);
    }

    mutex.unlock();
    return;
  }

  mediaObject->setState (REF);
  insertIntoChildren (mediaObject, childrenRefMap);
  mediaObjectsRefMap[mediaObject->id] = mediaObject;

  if (!mediaObject->getExcludeFromGC () ) {
    std::shared_ptr<KeepAliveData> data;

    data = std::shared_ptr<KeepAliveData> (new KeepAliveData() );
    data->mediaSet = this;
    data->objectId = mediaObject->id;

    mediaObjectsAlive[mediaObject->id] = data;
    keepAlive (*mediaObject);
  }

  mutex.unlock();
}

void
MediaSet::removeAutoRelease (const KmsMediaObjectId &id)
{
  std::shared_ptr<KeepAliveData> data;
  std::map<KmsMediaObjectId, std::shared_ptr<KeepAliveData>>::iterator it;

  mutex.lock();
  it = mediaObjectsAlive.find (id);

  if (it != mediaObjectsAlive.end() ) {
    data = it->second;

    if (data->timeoutId != 0) {
      g_source_remove (data->timeoutId);
    }
  }

  mutex.unlock();
}

void
MediaSet::releaseMediaObject (std::shared_ptr<MediaObjectImpl> mo)
{
  ObjectReleasing *obj = new ObjectReleasing();

  obj->object = mo;
  mo.reset();

  threadPool.push ([obj] () {
    delete obj;
  } );
}

void
MediaSet::unreg (const KmsMediaObjectRef &mediaObject, bool force)
{
  unregRecursive (mediaObject, force);
}

void
MediaSet::unreg (const KmsMediaObjectId &id, bool force)
{
  unregRecursive (id, force);
}

void
MediaSet::unregRecursive (const KmsMediaObjectRef &mediaObject, bool force,
                          bool rec)
{
  unregRecursive (mediaObject.id, force, rec);
}

void
MediaSet::unregRecursive (const KmsMediaObjectId &id, bool force, bool rec)
{
  std::shared_ptr<MediaObjectImpl> mo;

  mutex.lock();

  mo = mediaObjectToUnref (id);

  if (mo == NULL) {
    mutex.unlock();
    return;
  }

  auto childrenRefMapIt = childrenRefMap.find (id);

  if (childrenRefMapIt != childrenRefMap.end() ) {
    std::shared_ptr<std::set<KmsMediaObjectId>> childrenRef =
          childrenRefMapIt->second;

    if (force || mo->getUnregChilds () ) {
      childrenRefMap.erase (id);

      for (auto it = childrenRef->begin(); it != childrenRef->end(); it++) {
        unregRecursive (*it, force, true);
      }
    } else if (!childrenRef->empty () ) {
      GST_DEBUG ("MediaObject %" G_GINT64_FORMAT
                 " has children and not is forcing, so it will not be removed.", id);
      mutex.unlock();
      return;
    }
  }

  if (!rec) {
    if (mo->parent != NULL && mo->parent->getState () == UNREF) {
      unregRecursive (mo->parent->id, force);
    }
  }

  if (!force && !mo->getCollectOnUnreferenced () ) {
    mutex.unlock();
    return;
  }

  auto childrenUnrefMapIt = childrenUnrefMap.find (id);

  if (childrenUnrefMapIt != childrenUnrefMap.end() ) {
    std::shared_ptr<std::set<KmsMediaObjectId>> childrenUnref =
          childrenUnrefMapIt->second;
    childrenUnrefMap.erase (id);

    for (auto it = childrenUnref->begin(); it != childrenUnref->end(); it++) {
      unregRecursive (*it, true, true);
    }
  }

  mediaObjectsUnrefMap.erase (id);
  removeAutoRelease (mo->id);
  mediaObjectsAlive.erase (mo->id);

  GST_TRACE ("Release MediaObject %" G_GINT64_FORMAT, id);

  mutex.unlock();

  releaseMediaObject (mo);
}

int
MediaSet::size ()
{
  int size;

  mutex.lock();
  size = mediaObjectsRefMap.size();
  mutex.unlock();

  return size;
}

std::shared_ptr<MediaObjectImpl>
MediaSet::mediaObjectToRef (const KmsMediaObjectId &id)
{
  std::shared_ptr<MediaObjectImpl> mo = NULL;

  mutex.lock();

  auto refIf = mediaObjectsRefMap.find (id);

  if (refIf != mediaObjectsRefMap.end () ) {
    mo = refIf->second;
  } else {
    auto unrefIt = mediaObjectsUnrefMap.find (id);

    if (unrefIt != mediaObjectsUnrefMap.end () ) {
      mo = unrefIt->second;

      GST_TRACE ("MediaObject %" G_GINT64_FORMAT " to REF state", mo->id);
      mo->setState (REF);

      eraseFromChildren (mo, childrenUnrefMap);
      insertIntoChildren (mo, childrenRefMap);

      mediaObjectsUnrefMap.erase (id);
      mediaObjectsRefMap[id] = mo;
    }
  }

  mutex.unlock();

  return mo;
}

std::shared_ptr<MediaObjectImpl>
MediaSet::mediaObjectToUnref (const KmsMediaObjectId &id)
{
  std::shared_ptr<MediaObjectImpl> mo = NULL;

  mutex.lock();

  auto unrefIt = mediaObjectsUnrefMap.find (id);

  if (unrefIt != mediaObjectsUnrefMap.end () ) {
    mo = unrefIt->second;
  } else {
    auto refIt = mediaObjectsRefMap.find (id);

    if (refIt != mediaObjectsRefMap.end () ) {
      mo = refIt->second;

      GST_TRACE ("MediaObject %" G_GINT64_FORMAT " to UNREF state", mo->id);
      mo->setState (UNREF);

      eraseFromChildren (mo, childrenRefMap);
      insertIntoChildren (mo, childrenUnrefMap);

      mediaObjectsRefMap.erase (id);
      mediaObjectsUnrefMap[id] = mo;
      removeAutoRelease (id);
    }
  }

  mutex.unlock();

  return mo;
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

template std::shared_ptr<MixerEndPoint>
MediaSet::getMediaObject<MixerEndPoint> (const KmsMediaObjectRef &mediaObject);

template <class T> std::shared_ptr<T>
MediaSet::getMediaObject (const KmsMediaObjectRef &mediaObject)
throw (KmsMediaServerException)
{
  std::shared_ptr<MediaObjectImpl> mo;
  std::shared_ptr<T> typedMo;

  mo = mediaObjectToRef (mediaObject.id);

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
