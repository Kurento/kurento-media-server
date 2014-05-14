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

#ifndef __RABBITMQ_CONNECTION_HPP__
#define __RABBITMQ_CONNECTION_HPP__

#include <string>
#include <exception>
#include <functional>
#include <glibmm.h>
#include <amqp.h>

namespace kurento
{

class RabbitMQException : public std::exception
{

public:
  RabbitMQException (std::string desc) : desc (desc) {
  }
  virtual ~RabbitMQException() {}

  virtual const char *what() const throw() {
    return desc.c_str();
  }

private:
  std::string desc;
};

class RabbitMQTimeoutException : public RabbitMQException
{

public:
  RabbitMQTimeoutException (std::string desc) : RabbitMQException (desc) {
  }
  virtual ~RabbitMQTimeoutException() {}
};

class RabbitMQConnection
{
public:
  RabbitMQConnection (const std::string &address, int port);
  virtual ~RabbitMQConnection() throw ();

  int getFd();
  void declareQueue (const std::string &queue_name, bool durable = false,
                     int ttl = -1);
  void declareExchange (const std::string &queue_name, const std::string &type);
  void bindQueue (const std::string &queue_name,
                  const std::string &exchange_name);
  void consumeQueue (const std::string &queue_name, const std::string &tag);
  void readMessage (struct timeval *timeout,
                    std::function <void (const std::string &, std::string &) > process);
  void sendMessage (const std::string &message, const std::string &exchange,
                    const std::string &routingKey);

  static const std::string EXCHANGE_TYPE_DIRECT;
  static const std::string EXCHANGE_TYPE_FANOUT;
  static const std::string EXCHANGE_TYPE_TOPIC;
private:

  void sendMessage (const amqp_bytes_t &message, const amqp_bytes_t &exchange,
                    const amqp_bytes_t &routingKey,
                    const amqp_bytes_t &correlationID = amqp_empty_bytes);
  void sendReply (const amqp_envelope_t &envelope, const amqp_bytes_t &message);

  amqp_connection_state_t conn;
  amqp_socket_t *socket;
  Glib::RefPtr<Glib::IOSource> source;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __RABBITMQ_CONNECTION_HPP__ */
