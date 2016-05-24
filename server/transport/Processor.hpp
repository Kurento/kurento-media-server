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
  /**
   * Process the request
   *
   * @param request The request to be proccessed
   * @param response The response to be send
   * @param sessionId The sessionId associated with the channel that received
   *                  the request
   *
   * @returns The sessionId of the request
   */
  virtual std::string process (const std::string &request, std::string &response,
                               std::string &sessionId) = 0;

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
