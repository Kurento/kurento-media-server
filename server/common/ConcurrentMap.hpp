/*
 * ConcurrentMap.hpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
  Glib::Mutex mutex; // Protects the map
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
  V value;
  bool found = FALSE;

  mutex.lock();
  it = map.find (key);

  if (it != map.end() ) {
    found = true;
    value = it->second;
  } else {
    found = false;
  }

  mutex.unlock();

  if (!found) {
    throw MediaObjectNotFoundException();
  }

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
