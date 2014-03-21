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

/* This is included to avoid problems with slots and lamdas */
#include <type_traits>
#include <sigc++/sigc++.h>
namespace sigc
{
template <typename Functor>
struct functor_trait<Functor, false> {
  typedef decltype (::sigc::mem_fun (std::declval<Functor &> (),
                                     &Functor::operator() ) ) _intermediate;

  typedef typename _intermediate::result_type result_type;
  typedef Functor functor_type;
};
}

#define GST_CAT_DEFAULT kurento_media_set
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaSet"

#define COLLECTOR_INTERVAL 240

namespace kurento
{

class MediaSet::SessionTimeout
{
public:
  SessionTimeout (MediaSet &mediaSet,
                  const std::string &sessionId) : mediaSet (mediaSet) {
    this->sessionId = sessionId;

    start();
  }

  virtual ~SessionTimeout() {
    stop ();
  };

  void restart() {
    stop ();
    start ();
  };

private:

  bool removeSession() {
    GST_WARNING ("Timeout on session: %s", sessionId.c_str() );
    mediaSet.unrefSession (sessionId);
    return false;
  };

  void stop() {
    mediaSet.executeOnMainLoop ([&] {
      conn.disconnect();
      timeout_source->destroy();
      timeout_source.reset();
    });
  }

  void start() {
    timeout_source = Glib::TimeoutSource::create (COLLECTOR_INTERVAL * 1000);
    auto it = mediaSet.sessionTimeoutData.find (sessionId);

    if (it != mediaSet.sessionTimeoutData.end() ) {
      std::shared_ptr<SessionTimeout> sessionTimeout =  it->second;
      conn = timeout_source->connect ([sessionTimeout] () -> bool {
        GST_WARNING ("Timeout on session: %s", sessionTimeout->sessionId.c_str() );
        sessionTimeout->mediaSet.unrefSession (sessionTimeout->sessionId);
        return false;
      });
      timeout_source->attach (mediaSet.context);
    }
  };

  Glib::RefPtr<Glib::TimeoutSource> timeout_source;
  MediaSet &mediaSet;
  std::string sessionId;
  sigc::connection conn;
};

MediaSet &
MediaSet::getMediaSet()
{
  static MediaSet mediaSet;

  return mediaSet;
}

MediaSet::MediaSet()
{
  context = Glib::MainContext::create();
  loop = Glib::MainLoop::create (context, true);

  thread = Glib::Thread::create ([&] () {
    context->acquire();
    loop->run();
    GST_DEBUG ("Main loop thread stopped");
  });
}

MediaSet::~MediaSet ()
{
  Monitor monitor (mutex);

  terminated = true;

  if (!objectsMap.empty() ) {
    std::cerr << "Warning: Still " + std::to_string (objectsMap.size() ) +
              " object/s alive" << std::endl;
  }

  if (!sessionMap.empty() ) {
    std::cerr << "Warning: Still " + std::to_string (sessionMap.size() ) +
              " session/s alive" << std::endl;
  }

  if (!sessionTimeoutData.empty() ) {
    std::cerr << "Warning: Still " + std::to_string (sessionTimeoutData.size() ) +
              " session/s with timeout" << std::endl;
  }

  childrenMap.clear();
  sessionMap.clear();
  sessionTimeoutData.clear();

  if (Glib::Thread::self() != thread) {
    Glib::RefPtr<Glib::IdleSource> source = Glib::IdleSource::create ();
    source->connect (sigc::mem_fun (*this, &MediaSet::finishLoop) );
    source->attach (context);

    thread->join();
  } else {
    finishLoop();
  }

  threadPool.shutdown (true);
}

void
MediaSet::executeOnMainLoop (std::function<void () > func, bool force)
{
  if (terminated || !loop->is_running() ) {
    if (force) {
      func ();
    }

    return;
  }

  if (Glib::Thread::self() == thread) {
    func ();
  } else {
    Glib::Threads::Cond cond;
    Glib::Threads::Mutex mutex;
    Glib::Threads::Mutex::Lock lock (mutex);
    bool done = false;
    Glib::RefPtr<Glib::IdleSource> source = Glib::IdleSource::create ();


    source->connect ([&] ()->bool {
      Glib::Threads::Mutex::Lock lock (mutex);

      try {
        func ();
      } catch (...) {
      }

      done = true;
      cond.signal();

      return false;
    });

    source->attach (context);

    while (!done) {
      cond.wait (mutex);
    }
  }
}

bool
MediaSet::finishLoop ()
{
  loop->quit();

  return false;
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
    mediaObject =  std::shared_ptr<MediaObjectImpl> (mediaObjectPtr, [] (
    MediaObjectImpl * obj) {
      if (!MediaSet::getMediaSet().terminated) {
        MediaSet::getMediaSet().releasePointer (obj);
      } else {
        delete obj;
      }
    });
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

  keepAliveSession (sessionId);

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
  Monitor monitor (mutex);
  std::shared_ptr<SessionTimeout> data;

  auto it = sessionTimeoutData.find (sessionId);

  if (it == sessionTimeoutData.end() ) {
    GST_DEBUG ("Creating session: %s", sessionId.c_str() );
    data = std::shared_ptr<SessionTimeout> (new SessionTimeout (*this,
                                            sessionId) );
    sessionTimeoutData[sessionId] = data;
  } else {
    data = it->second;
    data->restart();
  }
}

void
MediaSet::releaseSession (const std::string &sessionId)
{
  // TODO
}

void
MediaSet::unrefSession (const std::string &sessionId)
{
  Monitor monitor (mutex);

  auto it = sessionMap.find (sessionId);

  if (it != sessionMap.end() ) {
    auto objects = it->second;

    for (auto it2 : objects) {
      unref (sessionId, it2.second);
    }
  }

  sessionMap.erase (sessionId);
  sessionTimeoutData.erase (sessionId);
}

void
MediaSet::unref (const std::string &sessionId,
                 std::shared_ptr< MediaObjectImpl > mediaObject)
{
  Monitor monitor (mutex);

  if (!mediaObject) {
    return;
  }

  auto it = sessionMap.find (sessionId);

  if (it != sessionMap.end() ) {
    auto it2 = it->second.find (mediaObject->getId() );

    if (it2 == it->second.end() ) {
      return;
    }

    it->second.erase (it2);

    if (it->second.size() == 0) {
      unrefSession (sessionId);
    }
  }

  auto it3 = reverseSessionMap.find (mediaObject->getId() );

  if (it3 != reverseSessionMap.end() ) {
    it3->second.erase (sessionId);

    if (it3->second.empty() ) {
      std::shared_ptr<MediaObjectImpl> parent;

      // Object has been removed from all the sessions, remove it from childrenMap
      childrenMap.erase (mediaObject->getId() );

      parent = std::dynamic_pointer_cast<MediaObjectImpl> (mediaObject->getParent() );

      if (parent) {
        childrenMap[parent->getId()].erase (mediaObject->getId() );
      }

      auto childrenIt = childrenMap.find (mediaObject->getId() );

      if (childrenIt != childrenMap.end() ) {
        auto childMap = childrenIt->second;

        for (auto child : childMap) {
          unref (sessionId, child.second);
        }
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
  Monitor monitor (mutex);

  objectsMap.erase (mediaObject->getId() );

  if (!terminated) {
    threadPool.push ( [mediaObject] () {
      GST_DEBUG ("Destroying %s", mediaObject->getIdStr().c_str() );
      delete mediaObject;
    });
  } else {
    GST_DEBUG ("Thread pool finished, destroying on the same thread %s",
               mediaObject->getIdStr().c_str() );
    delete mediaObject;
  }
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

std::shared_ptr< MediaObjectImpl >
MediaSet::getMediaObject (const std::string &sessionId,
                          const std::string &mediaObjectRef)
{
  std::shared_ptr< MediaObjectImpl > obj = getMediaObject (mediaObjectRef);

  ref (sessionId, obj);
  return obj;
}

std::shared_ptr< MediaObjectImpl >
MediaSet::getMediaObject (const std::string &sessionId,
                          const uint64_t &mediaObjectRef)
{
  std::shared_ptr< MediaObjectImpl > obj = getMediaObject (mediaObjectRef);

  ref (sessionId, obj);
  return obj;
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
