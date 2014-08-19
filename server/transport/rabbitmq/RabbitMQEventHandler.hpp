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

#ifndef __RABBITMQ_EVENT_HANDLER_HPP__
#define __RABBITMQ_EVENT_HANDLER_HPP__

#include <EventHandler.hpp>
#include "RabbitMQConnection.hpp"

namespace kurento
{

class RabbitMQEventHandler : public EventHandler
{
public:
  RabbitMQEventHandler (std::shared_ptr<MediaObjectImpl> obj,
                        const std::string &address, int port,
                        const std::string &exchange,
                        const std::string &routingKey);

  std::string getRoutingKey () {
    return routingKey;
  }

  virtual ~RabbitMQEventHandler();

  virtual void sendEvent (Json::Value &value);

private:
  RabbitMQConnection connection;
  std::string exchange;
  std::string routingKey;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __RABBITMQ_EVENT_HANDLER_HPP__ */
