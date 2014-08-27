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
#include <glibmm.h>
#include "RabbitMQListener.hpp"
#include "ExponentialBackoffStrategy.hpp"
#include <sys/types.h>
#include <unistd.h>

#define GST_CAT_DEFAULT kurento_rabbitmq_listener
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRabbitMQListener"

/* This is included to avoid problems with slots and lamdas */
#include <type_traits>
#include <sigc++/sigc++.h>
#include <event2/event_struct.h>

#define MAX_TIMEOUT (30 * 1000) /* 30 seconds */

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

namespace kurento
{

RabbitMQListener::RabbitMQListener ()
{
  timeoutStrategy = std::shared_ptr<RabbitMQReconnectStrategy>
                    (new ExponentialBackoffStrategy (MAX_TIMEOUT) );
};

RabbitMQListener::~RabbitMQListener()
{
  if (reconnectSrc) {
    reconnectSrc->destroy();
  }
}

void RabbitMQListener::setConfig (const std::string &address, int port)
{
  this->address = address;
  this->port = port;
}

void
RabbitMQListener::reconnect ()
{
  int timeout;

  timeout = timeoutStrategy->getTimeout();

  GST_DEBUG ("Reconnecting to RabbitMQ broker in %d ms.", timeout);

  reconnectSrc = Glib::TimeoutSource::create (timeout);
  reconnectSrc->connect ( [this] () -> bool {
    try {
      connection = std::shared_ptr<RabbitMQConnection> (new RabbitMQConnection (
        address, port) );
    } catch (Glib::IOChannelError &e) {
      reconnect ();
      return false;
    }

    timeoutStrategy->reset ();

    try {
      eventReconnectHandler ();
    } catch (std::bad_function_call &e) {
      /* No one is managing this event */
    }

    return false;
  });

  reconnectSrc->attach();
}

bool
RabbitMQListener::readMessages (Glib::IOCondition cond)
{
  if (cond & (Glib::IO_HUP | Glib::IO_NVAL | Glib::IO_ERR) ) {
    goto error;
  }

  readMessages();

  return true;

error:

  if (cond & Glib::IO_HUP) {
    GST_DEBUG ("Connection hung up");
    reconnect ();
  }

  return false;
}

bool
RabbitMQListener::readMessages ()
{
  struct timeval timeout;

  if (getpid () != pid) {
    return false;
  }

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  try {
    connection->readMessage (&timeout, [this] (RabbitMQMessage & message) {
      processMessage (message);
    });
  } catch (RabbitMQTimeoutException &e) {
    return false;
  } catch (RabbitMQException &e) {
    GST_ERROR ("%s", e.what() );
  }

  return true;
}

void RabbitMQListener::listenQueue (const std::string &queue, bool durable,
                                    int ttl)
{
  Glib::RefPtr<Glib::IdleSource> idle;
  int fd;

  if (connection == NULL) {
    connection = std::shared_ptr<RabbitMQConnection> (new RabbitMQConnection (
                   address, port) );
  }

  connection->declareQueue (queue, durable, ttl);
  connection->declareExchange (queue,
                               RabbitMQConnection::EXCHANGE_TYPE_DIRECT, durable, ttl);
  connection->bindQueue (queue, queue);
  connection->consumeQueue (queue, "");

  fd = connection->getFd();

  channel = Glib::IOChannel::create_from_fd (fd);
  channel->set_close_on_unref (false);
  channel->set_encoding ("");
  channel->set_buffered (false);

  source = channel->create_watch (Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR |
                                  Glib::IO_NVAL);

  // TODO: Investigate why std::bind cannot be used here
  source->connect ([this] (Glib::IOCondition cond) -> bool {
    return readMessages (cond);
  });

  source->attach (Glib::MainContext::get_default() );

  /* Add idle source to read all message that are already in the queue,
   * This is needed because the Channel callback is not being
   * triggered when the data is already in the socket when the main loop
   * starts */

  idle = Glib::IdleSource::create();

  idle->connect ([this] () -> bool {
    return readMessages ();
  });

  /* Only the same process that creates the channel can read it */
  pid = getpid ();
  idle->attach ();
}

void RabbitMQListener::stopListen ()
{
  channel->flush();
  source->destroy();
}

RabbitMQListener::StaticConstructor RabbitMQListener::staticConstructor;

RabbitMQListener::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
