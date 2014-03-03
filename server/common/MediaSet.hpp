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

#ifndef __MEDIA_SET_H__
#define __MEDIA_SET_H__

#include "types/MediaObjectImpl.hpp"

#include <glibmm.h>
#include <set>
#include <memory>

namespace kurento
{

typedef struct _KeepAliveData KeepAliveData;

class MediaObjectNotFound
{
};

class MediaSet
{
public:
  ~MediaSet ();

  /**
   * Set the state of the MediaObject as referenced and
   * register it in the MediaSet only if it is not into yet.
   */
  void reg (std::shared_ptr<MediaObjectImpl> mediaObject);
  void keepAlive (const uint64_t &mediaObjectRef);

  /**
   * Set the state of the MediaObject as unreferenced and
   * unregister it, and release it if possible.
   */
  void unreg (const uint64_t &mediaObjectRef, bool force = true);
  int size();

  std::shared_ptr<MediaObjectImpl> getMediaObject (const uint64_t
      &mediaObjectRef);

  static std::shared_ptr<MediaSet> getMediaSet();

private:
  MediaSet () {};

  Glib::Threads::RecMutex mutex;
  std::map<uint64_t, std::shared_ptr<MediaObjectImpl> >
  mediaObjectsRefMap, mediaObjectsUnrefMap;
  std::map<uint64_t, std::shared_ptr<std::set<uint64_t>> >
      childrenRefMap, childrenUnrefMap;
  std::map<uint64_t, std::shared_ptr<KeepAliveData>> mediaObjectsAlive;

  Glib::ThreadPool threadPool;

  void removeAutoRelease (const uint64_t &mediaObjectRef);

  std::shared_ptr<MediaObjectImpl> mediaObjectToRef (const uint64_t
      &mediaObjectRef);
  std::shared_ptr<MediaObjectImpl> mediaObjectToUnref (const uint64_t
      &mediaObjectRef);
  void releaseMediaObject (std::shared_ptr<MediaObjectImpl> mo);
  void unregRecursive (const uint64_t &mediaObjectRef, bool force = true,
                       bool rec = false);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __MEDIA_SET_H__ */
