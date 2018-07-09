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

#ifndef __CACHE_ENTRY_H__
#define __CACHE_ENTRY_H__

#include <glibmm.h>
#include <mutex>

#include <json/json.h>

namespace kurento
{

class CacheEntry
{
public:
  CacheEntry (unsigned int timeout, std::string sessionId, std::string requestId,
              Json::Value &response);
  Json::Value &getResponse (void);
  ~CacheEntry ();

  sigc::signal<void> signalTimeout;


private:
  Glib::RefPtr<Glib::TimeoutSource> source;
  std::recursive_mutex mutex;
  std::string sessionId;
  std::string requestId;
  Json::Value response;
  bool timedout;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __CACHE_ENTRY_H__ */
