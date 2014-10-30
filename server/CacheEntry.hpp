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

#ifndef __CACHE_ENTRY_H__
#define __CACHE_ENTRY_H__

#include <glibmm.h>
#include <mutex>

namespace kurento
{

class CacheEntry
{
public:
  CacheEntry (unsigned int timeout, std::string sessionId, int requestId,
              std::string response);
  std::string getResponse (void);
  ~CacheEntry ();

  sigc::signal<void> signalTimeout;


private:
  Glib::RefPtr<Glib::TimeoutSource> source;
  std::recursive_mutex mutex;
  std::string sessionId;
  int requestId;
  std::string response;
  bool timedout = false;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __CACHE_ENTRY_H__ */
