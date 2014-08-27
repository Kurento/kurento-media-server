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

#ifndef __RABBITMQ_LISTENER_HPP__
#define __RABBITMQ_LISTENER_HPP__

#include "RabbitMQConnection.hpp"
#include "RabbitMQReconnectStrategy.hpp"

namespace kurento
{

class RabbitMQListener
{
public:
  RabbitMQListener ();
  RabbitMQListener (const std::string &address, int port);
  virtual ~RabbitMQListener() throw ();

  void setConfig (const std::string &address, int port);
  void listenQueue (const std::string &queue, bool durable = false, int ttl = -1);
  void stopListen ();

protected:
  virtual void processMessage (RabbitMQMessage &message) = 0;

  std::shared_ptr <RabbitMQConnection> getConnection () {
    return connection;
  }

  void setEventReconnectHandler (std::function < void ( void ) > e) {
    eventReconnectHandler = e;
  }

private:
  bool readMessages (Glib::IOCondition cond);
  bool readMessages ();
  void reconnect ();

  Glib::RefPtr<Glib::IOChannel> channel;
  std::shared_ptr <RabbitMQConnection> connection;
  Glib::RefPtr<Glib::IOSource> source;

  Glib::RefPtr<Glib::TimeoutSource> reconnectSrc;

  std::string address;
  int port;

  int pid = 0;

  std::shared_ptr <RabbitMQReconnectStrategy> timeoutStrategy;

  std::function<void (void) > eventReconnectHandler;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __RABBITMQ_LISTENER_HPP__ */
