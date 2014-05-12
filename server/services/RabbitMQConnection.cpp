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

#include <gst/gst.h>
#include "RabbitMQConnection.hpp"
#include <amqp.h>
#include <amqp_tcp_socket.h>

#define GST_CAT_DEFAULT kurento_rabbitmq_connection
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRabbitMQConnection"

namespace kurento
{

static void
exception_on_error (amqp_rpc_reply_t x, const char *context)
{
  std::string ctx = context;

  switch (x.reply_type) {
  case AMQP_RESPONSE_NORMAL:
    return;

  case AMQP_RESPONSE_NONE:
    throw RabbitMQException (ctx + ": missing RPC reply type");

  case AMQP_RESPONSE_LIBRARY_EXCEPTION:
    if (AMQP_STATUS_TIMEOUT == x.library_error) {
      throw RabbitMQTimeoutException (ctx + ": Library exception: " +
                                      amqp_error_string2 (x.library_error) );
    } else {
      throw RabbitMQException (ctx + ": Library exception: " +
                               amqp_error_string2 (x.library_error) );
    }

  case AMQP_RESPONSE_SERVER_EXCEPTION:
    switch (x.reply.id) {
    case AMQP_CONNECTION_CLOSE_METHOD: {
      amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
      std::string message;
      gchar *error_message;

      error_message =
        g_strdup_printf ("%s: server connection error %d, message: %.*s\n", context,
                         m->reply_code, (int) m->reply_text.len, (char *) m->reply_text.bytes);

      message = error_message;
      g_free (error_message);

      throw RabbitMQException (message);
    }

    case AMQP_CHANNEL_CLOSE_METHOD: {
      amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
      std::string message;
      gchar *error_message;

      error_message = g_strdup_printf ("%s: server channel error %d, message: %.*s\n",
                                       context, m->reply_code, (int) m->reply_text.len, (char *) m->reply_text.bytes);

      message = error_message;
      g_free (error_message);

      throw RabbitMQException (message);
      break;
    }

    default:
      throw RabbitMQException (ctx + ": Channel Unknown Error");
      break;
    }

    break;
  }
}

RabbitMQConnection::RabbitMQConnection (const std::string &address, int port)
{
  conn = amqp_new_connection();

  socket = amqp_tcp_socket_new (conn);

  if (!socket) {
    throw Glib::IOChannelError (Glib::IOChannelError::Code::FAILED,
                                "Cannot create TCP socket");
  }

  if (amqp_socket_open (socket, address.c_str(), port) ) {
    throw Glib::IOChannelError (Glib::IOChannelError::Code::FAILED,
                                "Cannot open TCP socket");
  }

  exception_on_error (amqp_login (conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                                  "guest", "guest"), "Loging in");
  amqp_channel_open (conn, 1);
  exception_on_error (amqp_get_rpc_reply (conn), "Opening channel");
}

RabbitMQConnection::~RabbitMQConnection()
{
  if (conn == NULL) {
    GST_DEBUG ("service already stopped");
    return;
  }

  /* Errors are ignored during close */
  amqp_channel_close (conn, 1, AMQP_REPLY_SUCCESS);
  amqp_connection_close (conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection (conn);
  conn = NULL;
}

int
RabbitMQConnection::getFd()
{
  return amqp_socket_get_sockfd (socket);
}

void RabbitMQConnection::declareQueue (const std::string &queue_name)
{
  amqp_bytes_t queue = amqp_cstring_bytes (queue_name.c_str() );

  amqp_queue_declare (conn, 1,
                      queue, /* passive */ false, /* durable */ false, /* exclusive */
                      false, /* autodelete */ false, amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Declaring queue");
}

void RabbitMQConnection::declareExchange (const std::string &exchange_name,
    const std::string &type)
{
  amqp_bytes_t exchange = amqp_cstring_bytes (exchange_name.c_str() );
  amqp_bytes_t exchange_type = amqp_cstring_bytes (type.c_str() );

  amqp_exchange_declare (conn, 1, exchange, exchange_type,
                         /* passive */ false, /* durable */ false, amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Declaring exchange");
}

void
RabbitMQConnection::bindQueue (const std::string &queue_name,
                               const std::string &exchange_name)
{
  amqp_bytes_t queue = amqp_cstring_bytes (queue_name.c_str() );
  amqp_bytes_t exchange = amqp_cstring_bytes (exchange_name.c_str() );

  amqp_queue_bind (conn, 1, queue, exchange, amqp_empty_bytes,
                   amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Binding queue");
}

void
RabbitMQConnection::consumeQueue (const std::string &queue_name,
                                  const std::string &tag)
{
  amqp_bytes_t queue = amqp_cstring_bytes (queue_name.c_str() );
  amqp_bytes_t tag_id = amqp_cstring_bytes (tag.c_str() );

  amqp_basic_consume (conn, 1, queue, tag_id, /* no_local */ false,
                      /* no_ack */ false, /* exclusive */ false,
                      amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Consuming");
}

void
RabbitMQConnection::readMessage (struct timeval *timeout,
                                 std::function <void (const std::string &, std::string &) > process)
{
  amqp_envelope_t envelope;

  exception_on_error (amqp_consume_message (conn, &envelope, timeout, 0),
                      "Reading message");

  try {
    std::string response;
    std::string message (reinterpret_cast<char const *>
                         (envelope.message.body.bytes), envelope.message.body.len );

    process (message, response);

    amqp_basic_ack (conn, 1, envelope.delivery_tag, /* multiple */ false);

    try {
      if (envelope.message.properties.reply_to.len > 0) {
        sendReply (envelope, amqp_cstring_bytes (response.c_str() ) );
      }
    } catch (std::exception &e) {
      GST_ERROR ("Error sending reply: %s", e.what() );
    }

  } catch (...) {
    GST_WARNING ("Error processing message");
    amqp_basic_reject (conn, 1, envelope.delivery_tag, /* requeue */ true);
  }

  amqp_destroy_envelope (&envelope);
}

void
RabbitMQConnection::sendReply (const amqp_envelope_t &envelope,
                               const amqp_bytes_t &reply)
{
  std::string replyTo ( (char *) envelope.message.properties.reply_to.bytes,
                        envelope.message.properties.reply_to.len);

  sendMessage (reply, amqp_empty_bytes, envelope.message.properties.reply_to,
               envelope.message.properties.correlation_id);
}

void
RabbitMQConnection::sendMessage (const std::string &message,
                                 const std::string &exchange, const std::string &routingKey)
{
  sendMessage (amqp_cstring_bytes (message.c_str() ),
               amqp_cstring_bytes (exchange.c_str() ),
               amqp_cstring_bytes (routingKey.c_str() ) );
}

void
RabbitMQConnection::sendMessage (const amqp_bytes_t &message,
                                 const amqp_bytes_t &exchange, const amqp_bytes_t &routingKey,
                                 const amqp_bytes_t &correlationID)
{
  amqp_basic_properties_t props;
  int ret;

  props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
  props.content_type = amqp_cstring_bytes ("text/plain");
  props.delivery_mode = 2; /* persistent delivery mode */

  if (correlationID.len > 0) {
    props._flags |= AMQP_BASIC_CORRELATION_ID_FLAG;
    props.correlation_id = correlationID;
  }

  ret = amqp_basic_publish (conn, 1, exchange,
                            routingKey, /* mandatory */ false, /* inmediate */ false, &props, message);

  if (ret != AMQP_STATUS_OK) {
    GST_ERROR ("Error ret value: %d", ret);
  }
}

RabbitMQConnection::StaticConstructor RabbitMQConnection::staticConstructor;

RabbitMQConnection::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

const std::string RabbitMQConnection::EXCHANGE_TYPE_DIRECT ("direct");
const std::string RabbitMQConnection::EXCHANGE_TYPE_FANOUT ("fanout");
const std::string RabbitMQConnection::EXCHANGE_TYPE_TOPIC ("topic");

} /* kurento */
