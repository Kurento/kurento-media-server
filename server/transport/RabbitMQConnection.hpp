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

#include <memory>
#include <string>
#include <exception>
#include <functional>
#include <glibmm.h>
#include <amqp.h>

namespace kurento
{

class RabbitMQMessage;

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

class RabbitMQConnection : public
  std::enable_shared_from_this<RabbitMQConnection>
{
public:
  RabbitMQConnection (const std::string &address, int port);
  virtual ~RabbitMQConnection() throw ();

  std::string getAddress() {
    return address;
  }

  int getPort () {
    return port;
  }

  int getFd();
  void declareQueue (const std::string &queue_name, bool durable = false,
                     int ttl = -1);
  void deleteQueue (const std::string &queue_name, bool ifUnused = false,
                    bool ifEmpty = false);
  void declareExchange (const std::string &exchange_name, const std::string &type,
                        bool durable = false, const int ttl = -1);
  void deleteExchange (const std::string &exchange_name, bool ifUnused = false);
  void bindQueue (const std::string &queue_name,
                  const std::string &exchange_name);
  void consumeQueue (const std::string &queue_name, const std::string &tag);
  void readMessage (struct timeval *timeout,
                    std::function <void (RabbitMQMessage &) > process);
  void sendMessage (const std::string &message, const std::string &exchange,
                    const std::string &routingKey, const std::string &correlationID = "");

  void noCloseOnRelease() {
    closeOnRelease = false;
  }

  static const std::string EXCHANGE_TYPE_DIRECT;
  static const std::string EXCHANGE_TYPE_FANOUT;
  static const std::string EXCHANGE_TYPE_TOPIC;
private:

  void sendMessage (const amqp_bytes_t &message, const amqp_bytes_t &exchange,
                    const amqp_bytes_t &routingKey,
                    const amqp_bytes_t &correlationID = amqp_empty_bytes);
  void sendReply (const amqp_envelope_t &envelope, const amqp_bytes_t &message);

  std::string address;
  int port;

  bool closeOnRelease = true;
  amqp_connection_state_t conn;
  amqp_socket_t *socket;
  Glib::RefPtr<Glib::IOSource> source;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend RabbitMQMessage;
};

class RabbitMQMessage
{
public:
  RabbitMQMessage (std::shared_ptr <RabbitMQConnection> connection);
  virtual ~RabbitMQMessage() throw ();

  void reply (const std::string &response);
  void reply (std::shared_ptr <RabbitMQConnection> conn,
              const std::string &response);
  std::string getData();
  void ack();
  void noRejectOnRelease() {
    acked = true;
  }

private:

  bool acked = false;
  bool valid = false;
  amqp_envelope_t envelope;
  std::shared_ptr <RabbitMQConnection> connection;

  friend RabbitMQConnection;
};

} /* kurento */

#endif /* __RABBITMQ_CONNECTION_HPP__ */
