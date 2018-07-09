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

#ifndef __REQUEST_CACHE_H__
#define __REQUEST_CACHE_H__

#include <memory>
#include <map>
#include <mutex>

#include <json/json.h>

namespace kurento
{

class CacheEntry;

class RequestCache
{
public:
  RequestCache (unsigned int timeout);
  void addResponse (std::string sessionId, std::string requestId,
                    Json::Value &response);
  Json::Value getCachedResponse (std::string sessionId, std::string requestId);
  ~RequestCache ();

private:
  std::map<
      std::string,  // Session ID
      std::map<
          std::string,  // Request ID
          std::shared_ptr<CacheEntry>
      >
  > cache;
  std::recursive_mutex mutex;
  unsigned int timeout;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

class CacheException : public std::exception
{

public:
  CacheException (std::string desc) : desc (desc)
  {
  }
  virtual ~CacheException() {}
  virtual const char *what() const throw()
  {
    return desc.c_str();
  }

private:
  std::string desc;
};

} // kurento

#endif /* __REQUEST_CACHE__ */
