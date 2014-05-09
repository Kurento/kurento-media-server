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
#include "RabbitMQService.hpp"
#include <amqp.h>
#include <amqp_tcp_socket.h>

#define GST_CAT_DEFAULT kurento_rabbitmq_service
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRabbitMQService"

/* This is included to avoid problems with slots and lamdas */
#include <type_traits>
#include <sigc++/sigc++.h>
#include <event2/event_struct.h>
namespace sigc
{
template <typename Functor>
struct functor_trait<Functor, false> {
  typedef decltype (::sigc::mem_fun (std::declval<Functor &> (),
                                     &Functor::operator() ) ) _intermediate;

  typedef typename _intermediate::result_type result_type;
  typedef Functor functor_type;
};
}

/* Config file values */
#define RABBITMQ_GROUP "RabbitMQ"
#define RABBITMQ_SERVER_ADDRESS "serverAddress"
#define RABBITMQ_SERVER_PORT "serverPort"

/* Default config values */
#define RABBITMQ_SERVER_ADDRESS_DEFAULT "127.0.0.1"
#define RABBITMQ_SERVER_PORT_DEFAULT 5672

#define PIPELINE_CREATION "pipeline_creation"

static amqp_bytes_t queue_name = amqp_cstring_bytes (PIPELINE_CREATION);
static amqp_bytes_t exchange_name = amqp_cstring_bytes (PIPELINE_CREATION);

namespace kurento
{

class RabbitMQServiceException : public std::exception
{

public:
  RabbitMQServiceException (std::string desc) : desc (desc) {
  }
  virtual ~RabbitMQServiceException() {}

  virtual const char *what() const throw() {
    return desc.c_str();
  }

private:
  std::string desc;
};

class RabbitMQTimeoutException : public RabbitMQServiceException
{

public:
  RabbitMQTimeoutException (std::string desc) : RabbitMQServiceException (desc) {
  }
  virtual ~RabbitMQTimeoutException() {}
};

static void
exception_on_error (amqp_rpc_reply_t x, const char *context)
{
  std::string ctx = context;

  switch (x.reply_type) {
  case AMQP_RESPONSE_NORMAL:
    return;

  case AMQP_RESPONSE_NONE:
    throw RabbitMQServiceException (ctx + ": missing RPC reply type");

  case AMQP_RESPONSE_LIBRARY_EXCEPTION:
    if (AMQP_STATUS_TIMEOUT == x.library_error) {
      throw RabbitMQTimeoutException (ctx + ": Library exception: " +
                                      amqp_error_string2 (x.library_error) );
    } else {
      throw RabbitMQServiceException (ctx + ": Library exception: " +
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

      throw RabbitMQServiceException (message);
    }

    case AMQP_CHANNEL_CLOSE_METHOD: {
      amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
      std::string message;
      gchar *error_message;

      error_message = g_strdup_printf ("%s: server channel error %d, message: %.*s\n",
                                       context, m->reply_code, (int) m->reply_text.len, (char *) m->reply_text.bytes);

      message = error_message;
      g_free (error_message);

      throw RabbitMQServiceException (message);
      break;
    }

    default:
      throw RabbitMQServiceException (ctx + ": Channel Unknown Error");
      break;
    }

    break;
  }
}

static void
check_port (int port)
{
  if (port <= 0 || port > G_MAXUSHORT) {
    throw Glib::KeyFileError (Glib::KeyFileError::PARSE, "Invalid port value");
  }
}

RabbitMQService::RabbitMQService (Glib::KeyFile &confFile) : Service (confFile)
{
  try {
    address = confFile.get_string (RABBITMQ_GROUP, RABBITMQ_SERVER_ADDRESS);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Setting default address %s to media server",
                 RABBITMQ_SERVER_ADDRESS_DEFAULT);
    address = RABBITMQ_SERVER_ADDRESS_DEFAULT;
  }

  try {
    port = confFile.get_integer (RABBITMQ_GROUP, RABBITMQ_SERVER_PORT);
    check_port (port);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Setting default port %d to media server",
                 RABBITMQ_SERVER_PORT_DEFAULT);
    port = RABBITMQ_SERVER_PORT_DEFAULT;
  }
}

RabbitMQService::~RabbitMQService()
{
}

void
RabbitMQService::create_connection()
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

  amqp_queue_declare (conn, 1,
                      queue_name, /* passive */ false, /* durable */ false, /* exclusive */
                      false, /* autodelete */ false, amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Declaring queue");

  amqp_exchange_declare (conn, 1, amqp_cstring_bytes (PIPELINE_CREATION),
                         amqp_cstring_bytes ("fanout"),
                         /* passive */ false, /* durable */ false, amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Declaring exchange");

  amqp_queue_bind (conn, 1, queue_name, amqp_cstring_bytes (PIPELINE_CREATION),
                   queue_name,
                   amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Binding queue");

  amqp_basic_consume (conn, 1, queue_name,
                      amqp_empty_bytes, /* no_local */ false, /* no_ack */ false,
                      /* exclusive */ false, amqp_empty_table);
  exception_on_error (amqp_get_rpc_reply (conn), "Consuming");
}

bool
RabbitMQService::processMessages (Glib::IOCondition cond)
{
  amqp_envelope_t envelope;
  struct timeval timeout;

  if (cond & (Glib::IO_NVAL | Glib::IO_ERR | Glib::IO_HUP) ) {
    return false;
  }

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  try {
    exception_on_error (amqp_consume_message (conn, &envelope, &timeout, 0),
                        "Reading message");

    GST_DEBUG ("Message: %.*s", (int) envelope.message.body.len,
               (char *) envelope.message.body.bytes);
    // TODO: Process message
    amqp_basic_ack (conn, 1, envelope.delivery_tag, /* multiple */ false);
    amqp_destroy_envelope (&envelope);
  } catch (RabbitMQTimeoutException &e) {
  } catch (RabbitMQServiceException &e) {
    GST_ERROR ("%s", e.what() );
  }

  return true;
}

void RabbitMQService::start ()
{
  Glib::RefPtr<Glib::IOChannel> channel;
  int fd;

  create_connection();

  fd = amqp_socket_get_sockfd (socket);

  channel = Glib::IOChannel::create_from_fd (fd);
  channel->set_close_on_unref (false);
  channel->set_encoding ("");
  channel->set_buffered (false);

  source = channel->create_watch (Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR |
                                  Glib::IO_NVAL);

  // TODO: Investigate why std::bind cannot be used here
  source->connect ([this] (Glib::IOCondition cond) -> bool {
    return processMessages (cond);
  });

  source->attach (Glib::MainContext::get_default() );
}

void RabbitMQService::stop ()
{
  if (conn == NULL) {
    GST_DEBUG ("service already stopped");
    return;
  }

  source->destroy();
  /* Errors are ignored during close */
  amqp_channel_close (conn, 1, AMQP_REPLY_SUCCESS);
  amqp_connection_close (conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection (conn);
  conn = NULL;
  GST_DEBUG ("stop service");
}

RabbitMQService::StaticConstructor RabbitMQService::staticConstructor;

RabbitMQService::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
