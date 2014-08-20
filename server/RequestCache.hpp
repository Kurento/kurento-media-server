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

#ifndef __REQUEST_CACHE_H__
#define __REQUEST_CACHE_H__

#include <memory>
#include <map>
#include <mutex>

namespace kurento
{

class CacheEntry;

class RequestCache
{
public:
  RequestCache (unsigned int timeout);
  void addResponse (std::string sessionId, int requestId, std::string &response);
  std::string getCachedResponse (std::string sessionId, int requestId);
  ~RequestCache ();

private:
  std::map<std::string, std::map<int, std::shared_ptr <CacheEntry>>> cache;
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
  CacheException (std::string desc) : desc (desc) {
  }
  virtual ~CacheException() {}
  virtual const char *what() const throw() {
    return desc.c_str();
  }

private:
  std::string desc;
};

} // kurento

#endif /* __REQUEST_CACHE__ */
