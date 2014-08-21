/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#include "RequestCache.hpp"
#include <gst/gst.h>

#include "CacheEntry.hpp"

#define GST_CAT_DEFAULT kurento_request_cache
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRequestCache"

namespace kurento
{

RequestCache::RequestCache (unsigned int timeout)
{
  this->timeout = timeout;
}

RequestCache::~RequestCache ()
{
}

void
RequestCache::addResponse (std::string sessionId, int requestId,
                           std::string &response)
{
  std::shared_ptr<CacheEntry> entry;
  std::unique_lock<std::recursive_mutex> lock (mutex);

  entry = std::shared_ptr<CacheEntry> (new CacheEntry (timeout, sessionId,
                                       requestId, response) );

  cache[sessionId][requestId] = entry;
  entry->signalTimeout.connect ([this, sessionId, requestId] () {
    std::unique_lock<std::recursive_mutex> lock (mutex);
    auto it1 = this->cache.find (sessionId);

    if (it1 == this->cache.end() ) {
      return;
    }

    auto it2 = this->cache[sessionId].find (requestId);

    if (it2 == this->cache[sessionId].end() ) {
      return;
    }

    this->cache[sessionId].erase (it2);

    if (this->cache[sessionId].empty() ) {
      this->cache.erase (it1);
    }
  });
}

std::string
RequestCache::getCachedResponse (std::string sessionId, int requestId)
{
  std::map<int, std::shared_ptr <CacheEntry>> requests;

  std::unique_lock<std::recursive_mutex> lock (mutex);

  auto it1 = cache.find (sessionId);

  if (it1 == cache.end() ) {
    throw CacheException ("Session not cached");
  }

  requests = cache[sessionId];
  auto it2 = requests.find (requestId);

  if (it2 == requests.end() ) {
    throw CacheException ("Response not cached");
  }

  return requests[requestId]->getResponse();
}

RequestCache::StaticConstructor RequestCache::staticConstructor;

RequestCache::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
