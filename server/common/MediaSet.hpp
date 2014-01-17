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

namespace kurento
{

typedef struct _KeepAliveData KeepAliveData;

class MediaSet
{
public:
  MediaSet () {};
  ~MediaSet ();

  /**
   * Set the state of the MediaObject as referenced and
   * register it in the MediaSet only if it is not into yet.
   */
  void reg (std::shared_ptr<MediaObjectImpl> mediaObject);
  void keepAlive (const KmsMediaObjectRef &mediaObject) throw (
    KmsMediaServerException);

  /**
   * Set the state of the MediaObject as unreferenced and
   * unregister it, and release it if possible.
   */
  void unreg (const KmsMediaObjectRef &mediaObject, bool force = true);
  void unreg (const KmsMediaObjectId &id, bool force = true);
  int size();

  template <class T>
  std::shared_ptr<T> getMediaObject (const KmsMediaObjectRef &mediaObject) throw (
    KmsMediaServerException);

private:
  Glib::Threads::RecMutex mutex;
  std::map<KmsMediaObjectId, std::shared_ptr<MediaObjectImpl> >
  mediaObjectsRefMap, mediaObjectsUnrefMap;
  std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> >
      childrenRefMap, childrenUnrefMap;
  std::map<KmsMediaObjectId, std::shared_ptr<KeepAliveData>> mediaObjectsAlive;

  Glib::ThreadPool threadPool;

  bool canBeAutoreleased (const KmsMediaObjectRef &mediaObject);
  void removeAutoRelease (const KmsMediaObjectId &id);

  std::shared_ptr<MediaObjectImpl> mediaObjectToRef (const KmsMediaObjectId &id);
  std::shared_ptr<MediaObjectImpl> mediaObjectToUnref (const KmsMediaObjectId
      &id);
  void releaseMediaObject (std::shared_ptr<MediaObjectImpl> mo);
  void unregRecursive (const KmsMediaObjectRef &mediaObject, bool force = true,
                       bool rec = false);
  void unregRecursive (const KmsMediaObjectId &id, bool force = true,
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
