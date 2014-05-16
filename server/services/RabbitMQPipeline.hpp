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

#ifndef __RABBITMQ_PIPELINE_HPP__
#define __RABBITMQ_PIPELINE_HPP__

#include "RabbitMQListener.hpp"
#include <ServerMethods.hpp>

namespace kurento
{

class RabbitMQPipeline: private RabbitMQListener, private ServerMethods
{
public:
  RabbitMQPipeline (const std::string &address, const int port);
  virtual ~RabbitMQPipeline() throw ();
  virtual void startRequest (const std::string &request,
                             std::string &_response);
  virtual void stop ();

protected:
  virtual void processMessage (const std::string &message,
                               std::string &_response);

  virtual std::string connectEventHandler (std::shared_ptr<MediaObject> obj,
      const std::string &sessionId, const std::string &eventType,
      const Json::Value &params);

private:

  void destroyHandler (EventHandler *handler);

  std::map <std::string, std::weak_ptr <EventHandler>> handlers;
  Glib::Threads::Mutex mutex;
  std::string pipelineId;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __RABBITMQ_PIPELINE_HPP__ */
