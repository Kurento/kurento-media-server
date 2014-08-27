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
#include <Processor.hpp>
#include <mutex>

namespace kurento
{

class RabbitMQPipeline: private RabbitMQListener
{
public:
  RabbitMQPipeline (const boost::property_tree::ptree &config,
                    const std::string &address,
                    const int port, std::shared_ptr<Processor> processor);
  virtual ~RabbitMQPipeline() throw ();
  virtual void startRequest (RabbitMQMessage &message);

  virtual std::string processSubscription (std::shared_ptr<MediaObjectImpl> obj,
      const std::string &sessionId, const std::string &eventType,
      const Json::Value &params);

protected:
  virtual void processMessage (RabbitMQMessage &message);

private:

  void destroyHandler (EventHandler *handler);
  virtual void reconnect ();

  std::shared_ptr<Processor> processor;
  std::map <std::string, std::weak_ptr <EventHandler>> handlers;
  std::mutex mutex;
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
