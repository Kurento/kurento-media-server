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
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_media_set
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSet"

namespace kurento
{

std::shared_ptr< MediaSet >
MediaSet::getMediaSet()
{
  static std::shared_ptr<MediaSet> mediaSet (new MediaSet() );

  return mediaSet;
}

MediaSet::~MediaSet ()
{
  childrenMap.clear();
  sessionMap.clear();
}

std::shared_ptr<MediaObjectImpl>
MediaSet::ref (MediaObjectImpl *mediaObjectPtr)
{
  Monitor monitor (mutex);

  std::shared_ptr<MediaObjectImpl> mediaObject;

  if (mediaObjectPtr == NULL) {
    throw KurentoException ("Invalid object");
  }

  try {
    mediaObject = std::dynamic_pointer_cast<MediaObjectImpl>
                  (mediaObjectPtr->shared_from_this() );
  } catch (std::bad_weak_ptr e) {
    mediaObject =  std::shared_ptr<MediaObjectImpl> (mediaObjectPtr,
                   std::bind (
                     &MediaSet::releasePointer,
                     this,
                     std::placeholders::_1) );
  }

  objectsMap[mediaObject->getId()] = std::weak_ptr<MediaObjectImpl> (mediaObject);

  if (mediaObject->getParent() ) {
    std::shared_ptr<MediaObjectImpl> parent = std::dynamic_pointer_cast
        <MediaObjectImpl> (mediaObject->getParent() );

    ref (parent.get() );
    childrenMap[parent->getId()][mediaObject->getId()] = mediaObject;
  }

  return mediaObject;
}

void
MediaSet::ref (const std::string &sessionId,
               std::shared_ptr<MediaObjectImpl> mediaObject)
{
  Monitor monitor (mutex);

  if (mediaObject->getParent() ) {
    ref (sessionId,
         std::dynamic_pointer_cast<MediaObjectImpl> (mediaObject->getParent() ) );
  }

  sessionMap[sessionId][mediaObject->getId()] = mediaObject;
  reverseSessionMap[mediaObject->getId()].insert (sessionId);
  ref (mediaObject.get() );
}

void
MediaSet::keepAliveSession (const std::string &sessionId)
{
  // TODO:
}

void
MediaSet::releaseSession (const std::string &sessionId)
{
  // TODO
}

void
MediaSet::unrefSession (const std::string &sessionId)
{
  // TODO
}

void
MediaSet::unref (const std::string &sessionId,
                 std::shared_ptr< MediaObjectImpl > mediaObject)
{
  Monitor monitor (mutex);

  auto it = sessionMap.find (sessionId);

  if (it == sessionMap.end() ) {
    return;
  }

  auto it2 = it->second.find (mediaObject->getId() );

  if (it2 == it->second.end() ) {
    return;
  }

  it->second.erase (it2);

  if (it->second.size() == 0) {
    sessionMap.erase (it);
  }

  auto it3 = reverseSessionMap.find (mediaObject->getId() );

  if (it3 != reverseSessionMap.end() ) {
    it3->second.erase (sessionId);

    auto childrenIt = childrenMap.find (mediaObject->getId() );

    if (childrenIt != childrenMap.end() ) {
      auto childMap = childrenIt->second;

      for (auto child : childMap) {
        unref (sessionId, child.second);
      }
    }

    if (it3->second.size() == 0) {
      std::shared_ptr<MediaObjectImpl> parent;
      // Object has been removed from all the sessions, remove it from childrenMap
      childrenMap.erase (mediaObject->getId() );

      parent = std::dynamic_pointer_cast<MediaObjectImpl> (mediaObject->getParent() );

      if (parent) {
        childrenMap[parent->getId()].erase (mediaObject->getId() );
      }
    }
  }
}

void
MediaSet::unref (const std::string &sessionId,
                 const std::string &mediaObjectRef)
{
  std::shared_ptr <MediaObjectImpl> object;

  try {
    object = getMediaObject (mediaObjectRef);
  } catch (KurentoException e) {
    return;
  }

  unref (sessionId, object);
}

void
MediaSet::unref (const std::string &sessionId, const uint64_t &mediaObjectRef)
{
  std::shared_ptr <MediaObjectImpl> object;

  try {
    object = getMediaObject (mediaObjectRef);
  } catch (KurentoException e) {
    return;
  }

  unref (sessionId, object);
}

void MediaSet::releasePointer (MediaObjectImpl *mediaObject)
{
  GST_DEBUG ("Releasing media object %s", mediaObject->getIdStr().c_str() );
  mutex.lock();
  objectsMap.erase (mediaObject->getId() );
  mutex.unlock();

  threadPool.push ( [mediaObject] () {
    GST_DEBUG ("Detroying %s", mediaObject->getIdStr().c_str() );
    delete mediaObject;
  });
}

void MediaSet::release (std::shared_ptr< MediaObjectImpl > mediaObject)
{
  Monitor monitor (mutex);

  auto it = reverseSessionMap.find (mediaObject->getId() );

  if (it == reverseSessionMap.end() ) {
    /* Already released */
    return;
  }

  auto sessions = it->second;

  for (auto it2 : sessions) {
    unref (it2, mediaObject);
  }

  objectsMap.erase (mediaObject->getId() );
}

void MediaSet::release (const std::string &mediaObjectRef)
{
  try {
    std::shared_ptr< MediaObjectImpl > obj = getMediaObject (mediaObjectRef);

    release (obj);
  } catch (...) {
    /* Do not raise exception if it is already released*/
  }
}

void MediaSet::release (const uint64_t &mediaObjectRef)
{
  try {
    std::shared_ptr< MediaObjectImpl > obj = getMediaObject (mediaObjectRef);

    release (obj);
  } catch (...) {
    /* Do not raise exception if it is already released*/
  }
}

std::shared_ptr< MediaObjectImpl >
MediaSet::getMediaObject (const std::string &mediaObjectRef)
{
  uint64_t objectRef;

  try {
#if __WORDSIZE == 64
    objectRef = std::stoul (mediaObjectRef);
#else
    objectRef = std::stoull (mediaObjectRef);
#endif
  } catch (...) {
    throw KurentoException ("Invalid object reference");
  }

  return getMediaObject (objectRef);
}

std::shared_ptr< MediaObjectImpl >
MediaSet::getMediaObject (const uint64_t &mediaObjectRef)
{
  std::shared_ptr <MediaObjectImpl> objectLocked;
  Monitor monitor (mutex);

  auto it = objectsMap.find (mediaObjectRef);

  if (it == objectsMap.end() ) {
    throw KurentoException ("Object not found");
  }

  try {
    objectLocked = it->second.lock();
  } catch (...) {
    throw KurentoException ("Object not found");
  }

  if (!objectLocked) {
    throw KurentoException ("Object not found");
  }

  return objectLocked;
}

MediaSet::Monitor::Monitor (Glib::Threads::RecMutex &mutex) : mutex (mutex)
{
  mutex.lock();
}

MediaSet::Monitor::~Monitor()
{
  mutex.unlock();
}

void MediaSet::deleter (MediaObjectImpl *mo)
{
  GST_ERROR ("Deleting");
}

MediaSet::StaticConstructor MediaSet::staticConstructor;

MediaSet::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
