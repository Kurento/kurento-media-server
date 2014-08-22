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

#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

#include <MediaObjectImpl.hpp>

namespace kurento
{

class Processor
{
public:
  Processor () {};
  virtual ~Processor() throw () {};
  virtual void process (const std::string &request, std::string &response) = 0;

  virtual void keepAliveSession (const std::string &sessionId) = 0;
  virtual void setEventSubscriptionHandler (std::function < std::string (
        std::shared_ptr<MediaObjectImpl> obj,
        const std::string &sessionId, const std::string &eventType,
        const Json::Value &params) > eventSubscriptionHandler) = 0;
  virtual std::string connectEventHandler (std::shared_ptr<MediaObjectImpl> obj,
      const std::string &sessionId, const std::string &eventType,
      std::shared_ptr<EventHandler> handler) = 0;
  virtual void registerEventHandler (std::shared_ptr<MediaObjectImpl> obj,
                                     const std::string &sessionId, const  std::string &subscriptionId,
                                     std::shared_ptr<EventHandler> handler) = 0;
};

} /* kurento */

#endif /* __PROCESSOR_HPP__ */
