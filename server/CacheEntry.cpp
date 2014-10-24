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

#include "CacheEntry.hpp"
#include <gst/gst.h>

#define GST_CAT_DEFAULT kurento_cache_entry
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoCacheEntry"

using namespace Glib::Threads;

/* Next stuff is included to avoid problems with slots and lamdas */
#include <type_traits>
#include <sigc++/sigc++.h>
#include <event2/event_struct.h>
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

namespace kurento
{

CacheEntry::CacheEntry (unsigned int timeout, std::string sessionId,
                        int requestId, std::string response)
{
  this->response = response;
  this->requestId = requestId;
  this->sessionId = sessionId;

  source = Glib::TimeoutSource::create (timeout);
  source->connect ( [this] () -> bool {
    std::unique_lock<std::recursive_mutex> lock (mutex);
    this->timedout = true;
    lock.unlock();

    this->signalTimeout.emit();

    return false;
  });

  source->attach();
}

std::string
CacheEntry::getResponse (void)
{
  return response;
}

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
