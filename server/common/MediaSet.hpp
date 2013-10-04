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

class MediaSet
{
public:
  MediaSet () {};

  void put (std::shared_ptr<MediaObjectImpl> mediaObject);
  void remove (const KmsMediaObjectRef &mediaObject);
  void remove (const KmsMediaObjectId &id);
  int size();

  template <class T>
  std::shared_ptr<T> getMediaObject (const KmsMediaObjectRef &mediaObject);

private:
  Glib::Threads::RecMutex mutex;
  std::map<KmsMediaObjectId, std::shared_ptr<MediaObjectImpl> > mediaObjectsMap;
  std::map<KmsMediaObjectId, std::shared_ptr<std::set<KmsMediaObjectId>> > childrenMap;
};

} // kurento

#endif /* __MEDIA_SET_H__ */
