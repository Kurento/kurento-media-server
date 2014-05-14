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
#include "RabbitMQPipeline.hpp"

#define GST_CAT_DEFAULT kurento_rabbitmq_listener
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRabbitMQPipeline"

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

// #define PIPELINE_QUEUE_PREFIX "media_pipeline_"
#define PIPELINE_QUEUE_PREFIX ""
#define EVENT_EXCHANGE_PREFIX "event_"

#define PIPELINE_QUEUE_TTL 240000

namespace kurento
{

RabbitMQPipeline::RabbitMQPipeline (const std::string &address, const int port)
{
  setConfig (address, port);
}

RabbitMQPipeline::~RabbitMQPipeline()
{
}

void
RabbitMQPipeline::processMessage (const std::string &message,
                                  std::string &_response)
{
  GST_DEBUG ("Message: >%s<", message.c_str() );
  process (message, _response);
  GST_DEBUG ("Response: >%s<", _response.c_str() );
}

void
RabbitMQPipeline::startRequest (const std::string &request,
                                std::string &_response)
{
  Json::Value responseJson;
  Json::Reader reader;

  GST_DEBUG ("Message: >%s<", request.c_str() );
  process (request, _response);

  reader.parse (_response, responseJson);

  if (responseJson.isObject() && responseJson.isMember ("result")
      && responseJson["result"].isObject()
      && responseJson["result"].isMember ("value") ) {
    std::string id = responseJson["result"]["value"].asString();

    listenQueue (PIPELINE_QUEUE_PREFIX + id, false, PIPELINE_QUEUE_TTL);
    getConnection()->declareExchange (EVENT_EXCHANGE_PREFIX + id,
                                      RabbitMQConnection::EXCHANGE_TYPE_FANOUT);
  }

  GST_DEBUG ("Response: >%s<", _response.c_str() );
}

void
RabbitMQPipeline::stop ()
{
  stopListen();
}

RabbitMQPipeline::StaticConstructor RabbitMQPipeline::staticConstructor;

RabbitMQPipeline::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
