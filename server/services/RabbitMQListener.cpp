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

#define GST_CAT_DEFAULT kurento_rabbitmq_listener
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRabbitMQListener"

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

namespace kurento
{

RabbitMQListener::~RabbitMQListener()
{
}

void RabbitMQListener::setConfig (const std::string &address, int port)
{
  this->address = address;
  this->port = port;
}

bool
RabbitMQListener::readMessages (Glib::IOCondition cond)
{
  struct timeval timeout;

  if (cond & (Glib::IO_NVAL | Glib::IO_ERR | Glib::IO_HUP) ) {
    return false;
  }

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  try {
    connection->readMessage (&timeout, [this] (RabbitMQMessage & message) {
      processMessage (message);
    });
  } catch (RabbitMQTimeoutException &e) {
  } catch (RabbitMQException &e) {
    GST_ERROR ("%s", e.what() );
  }

  return true;
}

void RabbitMQListener::listenQueue (const std::string &queue, bool durable,
                                    int ttl)
{
  Glib::RefPtr<Glib::IOChannel> channel;
  int fd;

  connection = std::shared_ptr<RabbitMQConnection> (new RabbitMQConnection (
                 address, port) );
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
}

void RabbitMQListener::stopListen ()
{
  source->destroy();
}

RabbitMQListener::StaticConstructor RabbitMQListener::staticConstructor;

RabbitMQListener::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
