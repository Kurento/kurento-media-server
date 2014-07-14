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

namespace kurento
{

CacheEntry::CacheEntry (unsigned int timeout, std::string sessionId,
                        int requestId, std::string response)
{
  this->response = response;
  this->requestId = requestId;
  this->sessionId = sessionId;

  /* TODO: Add timeout callback */
}

CacheEntry::~CacheEntry ()
{
  GST_DEBUG ("Do something");
}

CacheEntry::StaticConstructor CacheEntry::staticConstructor;

CacheEntry::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento