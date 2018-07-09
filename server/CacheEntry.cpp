/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "CacheEntry.hpp"
#include <gst/gst.h>
#include <config.h>

#define GST_CAT_DEFAULT kurento_cache_entry
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoCacheEntry"

using namespace Glib::Threads;

/* Next stuff is included to avoid problems with slots and lamdas */
#include <type_traits>
#include <sigc++/sigc++.h>
#include <event2/event_struct.h>
#if FIX_SIGC
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
#endif

namespace kurento
{

CacheEntry::CacheEntry (unsigned int timeout, std::string sessionId,
                        std::string requestId, Json::Value &response)
{
  this->sessionId = sessionId;
  this->requestId = requestId;
  this->response = response;
  this->timedout = false;

  this->source = Glib::TimeoutSource::create (timeout);
  source->connect ( [this] () -> bool {
    std::unique_lock<std::recursive_mutex> lock (this->mutex);
    this->timedout = true;
    lock.unlock();

    this->signalTimeout.emit();

    return false;
  });

  source->attach();
}

Json::Value &CacheEntry::getResponse() { return response; }

CacheEntry::~CacheEntry ()
{
  std::unique_lock<std::recursive_mutex> lock (mutex);

  if (!timedout) {
    source->destroy();
  }
}

CacheEntry::StaticConstructor CacheEntry::staticConstructor;

CacheEntry::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
