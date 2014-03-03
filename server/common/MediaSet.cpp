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

#include "utils/utils.hpp"
#include <gst/gst.h>

#define GST_CAT_DEFAULT kurento_media_set
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSet"

namespace kurento
{

struct _KeepAliveData {
  guint timeoutId;
  MediaSet *mediaSet;
  uint64_t objectId;
};

std::shared_ptr< MediaSet >
MediaSet::getMediaSet()
{
  static std::shared_ptr<MediaSet> mediaSet (new MediaSet() );

  return mediaSet;
}


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
                 "waiting %d ms", object->getId(), wait_times * 50);
      g_usleep (wait_times++ * 50000);
    }

    GST_DEBUG ("Destroying object %" G_GINT64_FORMAT, object->getId() );

    object.reset();
  }

  std::shared_ptr <MediaObjectImpl> object;
};

static void
eraseFromChildren (std::shared_ptr<MediaObjectImpl> mo,
                   std::map<uint64_t, std::shared_ptr<std::set<uint64_t>> >
                   &childrenMap)
{
  std::shared_ptr<MediaObjectImpl> parent =
    std::dynamic_pointer_cast<MediaObjectImpl> (mo->getParent() );

  if (parent == NULL) {
    return;
  }

  auto it = childrenMap.find (parent->getId() );

  if (it != childrenMap.end() ) {
    std::shared_ptr<std::set<uint64_t>> children;

    children = it->second;
    children->erase (mo->getId() );
  }
}

static void
insertIntoChildren (std::shared_ptr<MediaObjectImpl> mo,
                    std::map<uint64_t, std::shared_ptr<std::set<uint64_t>> >
                    &childrenMap)
{
  std::shared_ptr<std::set<uint64_t>> children;
  std::shared_ptr<MediaObjectImpl> parent;

  if (mo->getParent() == NULL) {
    return;
  }

  parent = std::dynamic_pointer_cast<MediaObjectImpl> (mo->getParent() );

  auto it = childrenMap.find (parent->getId() );

  if (it != childrenMap.end() ) {
    children = it->second;
  } else {
    children = std::shared_ptr<std::set<uint64_t>> (new
               std::set<uint64_t>() );
    childrenMap[parent->getId()] = children;
  }

  children->insert (mo->getId() );
}

MediaSet::~MediaSet ()
{
  threadPool.shutdown (true);
}

void
MediaSet::keepAlive (const uint64_t &mediaObject)
{
  std::shared_ptr<MediaObjectImpl> mo;
  std::shared_ptr<KeepAliveData> data;
  std::map<uint64_t, std::shared_ptr<KeepAliveData>>::iterator it;

  /* Check that object exists and it is not exluded from GC */
  mo = getMediaObject (mediaObject);

  if (mo->getGarbagePeriod () == 0 ) {
    GST_DEBUG ("MediaObject %" G_GINT64_FORMAT " is excluded from GC",
               mediaObject);
    return;
  }

  mutex.lock();
  it = mediaObjectsAlive.find (mediaObject);

  if (it != mediaObjectsAlive.end() ) {
    data = it->second;

    if (data->timeoutId != 0) {
      g_source_remove (data->timeoutId);
    }

    data->timeoutId = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT,
                      mo->getGarbagePeriod() * 2, keep_alive_time_out,
                      (gpointer) data.get (), NULL);
  }

  mutex.unlock();
}

void
MediaSet::reg (std::shared_ptr<MediaObjectImpl> mediaObject)
{
  std::shared_ptr<MediaObjectImpl> mo;

  mutex.lock();

  mo = mediaObjectToRef (mediaObject->getId() );

  if (mo != NULL) {
    if (mediaObject.get () != mo.get () ) {
      GST_ERROR ("There is another MediaObject registered "
                 "with the same id (%" G_GINT64_FORMAT ")", mo->getId() );
    } else {
      GST_DEBUG ("MediaObject %" G_GINT64_FORMAT " is already in the MediaSet",
                 mo->getId() );
    }

    mutex.unlock();
    return;
  }

  mediaObject->setState (MediaObjectImpl::REF);
  insertIntoChildren (mediaObject, childrenRefMap);
  mediaObjectsRefMap[mediaObject->getId()] = mediaObject;

  if (mediaObject->getGarbagePeriod () != 0) {
    std::shared_ptr<KeepAliveData> data;

    data = std::shared_ptr<KeepAliveData> (new KeepAliveData() );
    data->mediaSet = this;
    data->objectId = mediaObject->getId();

    mediaObjectsAlive[mediaObject->getId()] = data;
    keepAlive (mediaObject->getId() );
  }

  mutex.unlock();
}

void
MediaSet::removeAutoRelease (const uint64_t &id)
{
  std::shared_ptr<KeepAliveData> data;
  std::map<uint64_t, std::shared_ptr<KeepAliveData>>::iterator it;

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
MediaSet::unreg (const uint64_t &id, bool force)
{
  unregRecursive (id, force);
}

void
MediaSet::unregRecursive (const uint64_t &id, bool force, bool rec)
{
  std::shared_ptr<MediaObjectImpl> mo;
  std::shared_ptr<MediaObjectImpl> parent;

  mutex.lock();

  mo = mediaObjectToUnref (id);

  if (mo == NULL) {
    mutex.unlock();
    return;
  }

  auto childrenRefMapIt = childrenRefMap.find (id);

  if (childrenRefMapIt != childrenRefMap.end() ) {
    std::shared_ptr<std::set<uint64_t>> childrenRef =
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

  parent = std::dynamic_pointer_cast<MediaObjectImpl> (mo->getParent() );

  if (!rec) {
    if (parent != NULL
        && parent->getState () == MediaObjectImpl::UNREF) {
      unregRecursive (parent->getId(), force);
    }
  }

  if (!force && mo->getGarbagePeriod () == 0 ) {
    mutex.unlock();
    return;
  }

  auto childrenUnrefMapIt = childrenUnrefMap.find (id);

  if (childrenUnrefMapIt != childrenUnrefMap.end() ) {
    std::shared_ptr<std::set<uint64_t>> childrenUnref =
                                       childrenUnrefMapIt->second;
    childrenUnrefMap.erase (id);

    for (auto it = childrenUnref->begin(); it != childrenUnref->end(); it++) {
      unregRecursive (*it, true, true);
    }
  }

  mediaObjectsUnrefMap.erase (id);
  removeAutoRelease (mo->getId() );
  mediaObjectsAlive.erase (mo->getId() );

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
MediaSet::mediaObjectToRef (const uint64_t &id)
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

      GST_TRACE ("MediaObject %" G_GINT64_FORMAT " to REF state", mo->getId() );
      mo->setState (MediaObjectImpl::REF);

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
MediaSet::mediaObjectToUnref (const uint64_t &id)
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

      GST_TRACE ("MediaObject %" G_GINT64_FORMAT " to UNREF state", mo->getId() );
      mo->setState (MediaObjectImpl::REF);

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

std::shared_ptr<MediaObjectImpl>
MediaSet::getMediaObject (const uint64_t &mediaObject)
{
  std::shared_ptr<MediaObjectImpl> mo;

  mo = mediaObjectToRef (mediaObject);

  if (mo == NULL) {
    MediaObjectNotFound except;
    throw except;
  }

  return mo;
}

MediaSet::StaticConstructor MediaSet::staticConstructor;

MediaSet::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
