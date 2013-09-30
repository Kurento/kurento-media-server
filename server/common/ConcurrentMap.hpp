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

#ifndef __CONCURRENT_MAP_H__
#define __CONCURRENT_MAP_H__

#include "mediaServer_types.h"

#include <glibmm.h>

namespace kurento
{

template <class K, class V>
class ConcurrentMap
{
public:
  ConcurrentMap () {};

  void put (K key, V value);
  void remove (K key);
  V getValue (K key);
  int size();

private:
  Glib::Threads::RecMutex mutex; /* Protects the map */
  std::map<K, V> map;
};

template <class K, class V>
void
ConcurrentMap<K, V>::put (K key, V value)
{
  mutex.lock();
  map[key] = value;
  mutex.unlock();
}

template <class K, class V>
void
ConcurrentMap<K, V>::remove (K key)
{
  mutex.lock();
  map.erase (key);
  mutex.unlock();
}

template <class K, class V>
V
ConcurrentMap<K, V>::getValue (K key)
{
  typename std::map<K, V>::iterator it;
  V value = NULL;

  mutex.lock();
  it = map.find (key);

  if (it != map.end() ) {
    value = it->second;
  }

  mutex.unlock();

  return value;
}

template <class K, class V>
int
ConcurrentMap<K, V>::size ()
{
  int ret;

  mutex.lock();
  ret = map.size();
  mutex.unlock();

  return ret;
}

} // kurento

#endif /* __CONCURRENT_MAP_H__ */
