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

#ifndef __MEDIA_HANDLER_H__
#define __MEDIA_HANDLER_H__

#include "mediaHandler_types.h"
#include <glibmm.h>

namespace kurento
{

class MediaHandlerAddress
{
public:
  MediaHandlerAddress (const std::string &address, const int32_t port)
  {
    this->address = address;
    this->port= port;
  };

  std::string address;
  int32_t port;
};

class MediaHandler
{
public:
  MediaHandler (int32_t id)
  {
    this->id= id;
  };

  void addAddress (std::shared_ptr<MediaHandlerAddress> &address) {
    mutex.lock();
    addresses.push_back (address);
    mutex.unlock();
  }
// TODO: reuse when needed
#if 0
  void sendEvent (MediaEvent &event);
#endif

private:
  Glib::Threads::RecMutex mutex;
  int32_t id;
  std::list<std::shared_ptr<MediaHandlerAddress>> addresses;

};

} // kurento

#endif /* __MEDIA_HANDLER_H__ */
