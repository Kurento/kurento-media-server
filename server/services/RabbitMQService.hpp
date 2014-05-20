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

#ifndef __RABBITMQ_SERVICE_HPP__
#define __RABBITMQ_SERVICE_HPP__

#include "Service.hpp"
#include "HttpService.hpp"
#include "RabbitMQListener.hpp"
#include "RabbitMQPipeline.hpp"

namespace kurento
{

class RabbitMQService: private RabbitMQListener, public Service
{
public:
  RabbitMQService (Glib::KeyFile &confFile);
  virtual ~RabbitMQService() throw ();
  virtual void start ();
  virtual void stop ();

protected:
  virtual void processMessage (RabbitMQMessage &message);

private:
  std::list <std::shared_ptr<RabbitMQPipeline>> pipelines;

  std::string address;
  int port;

  Glib::KeyFile confFile;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __RABBITMQ_SERVICE_HPP__ */
