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

#ifndef __RABBITMQ_TRANSPORT_HPP__
#define __RABBITMQ_TRANSPORT_HPP__

#include "Transport.hpp"
#include "RabbitMQListener.hpp"
#include "RabbitMQPipeline.hpp"
#include <SignalHandler.hpp>

namespace kurento
{

class RabbitMQTransport: private RabbitMQListener, public Transport
{
public:
  RabbitMQTransport (const boost::property_tree::ptree &config,
                     std::shared_ptr<Processor> processor);
  virtual ~RabbitMQTransport() throw ();
  virtual void start ();
  virtual void stop ();

protected:
  virtual void processMessage (RabbitMQMessage &message);

private:
  void childSignal (uint32_t signal);
  void reconnect ();

  std::shared_ptr<Processor> processor;
  std::list <int> childs;
  std::shared_ptr<RabbitMQPipeline> pipeline;

  std::string address;
  int port;

  const boost::property_tree::ptree &config;

  std::shared_ptr <SignalHandler> signalHandler;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __RABBITMQ_TRANSPORT_HPP__ */
