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
#include "RabbitMQEventHandler.hpp"

#include <MediaObjectImpl.hpp>
#include <MediaSet.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#define GST_CAT_DEFAULT kurento_rabbitmq_pipeline
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

static std::string
generateUUID()
{
  std::stringstream ss;
  boost::uuids::uuid uuid = boost::uuids::random_generator() ();

  ss << uuid;
  return ss.str();
}

RabbitMQPipeline::RabbitMQPipeline (const boost::property_tree::ptree &config,
                                    const std::string &address, const int port,
                                    std::shared_ptr<Processor> processor) : processor (processor)
{
  setConfig (address, port);

  MediaSet::getMediaSet()->signalEmpty.connect ([] () {
    GST_INFO ("Mediaset is empty, exiting from child process");
    kill (getpid(), SIGINT);
  });

  processor->setEventSubscriptionHandler (std::bind (
      &RabbitMQPipeline::processSubscription, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) );

  setEventReconnectHandler (std::bind (&RabbitMQPipeline::reconnect, this) );
}

RabbitMQPipeline::~RabbitMQPipeline()
{
  stopListen();

  if (!pipelineId.empty() ) {
    getConnection()->deleteQueue (PIPELINE_QUEUE_PREFIX + pipelineId);
    getConnection()->deleteExchange (PIPELINE_QUEUE_PREFIX + pipelineId);
    getConnection()->deleteExchange (EVENT_EXCHANGE_PREFIX + pipelineId);
  }
}

void
RabbitMQPipeline::processMessage (RabbitMQMessage &message)
{
  std::string data = message.getData();
  std::string response;

  GST_DEBUG ("Message: >%s<", data.c_str() );
  processor->process (data, response);
  GST_DEBUG ("Response: >%s<", response.c_str() );

  message.ack();
  message.reply (response);
}

void
RabbitMQPipeline::reconnect ()
{
  GST_DEBUG ("Reconnected");
  listenQueue (PIPELINE_QUEUE_PREFIX + pipelineId, false, PIPELINE_QUEUE_TTL);
  getConnection()->declareExchange (EVENT_EXCHANGE_PREFIX + pipelineId,
                                    RabbitMQConnection::EXCHANGE_TYPE_FANOUT,
                                    false, PIPELINE_QUEUE_TTL);
}

void
RabbitMQPipeline::startRequest (RabbitMQMessage &message)
{
  Json::Value responseJson;
  Json::Reader reader;
  std::string response;
  std::string request = message.getData();

  GST_DEBUG ("Message: >%s<", request.c_str() );
  processor->process (request, response);

  reader.parse (response, responseJson);
  message.ack();

  if (responseJson.isObject() && responseJson.isMember ("result")
      && responseJson["result"].isObject()
      && responseJson["result"].isMember ("value") ) {
    pipelineId = responseJson["result"]["value"].asString();

    listenQueue (PIPELINE_QUEUE_PREFIX + pipelineId, false, PIPELINE_QUEUE_TTL);
    getConnection()->declareExchange (EVENT_EXCHANGE_PREFIX + pipelineId,
                                      RabbitMQConnection::EXCHANGE_TYPE_FANOUT,
                                      false, PIPELINE_QUEUE_TTL);
  }

  message.reply (getConnection(), response);

  GST_DEBUG ("Response: >%s<", response.c_str() );

  if (MediaSet::getMediaSet()->empty() ) {
    GST_ERROR ("Error creating media pipeline, terminating process");
    kill (getpid(), SIGINT);
  }
}

std::string
RabbitMQPipeline::processSubscription (std::shared_ptr< MediaObjectImpl > obj,
                                       const std::string &sessionId,
                                       const std::string &eventType,
                                       const Json::Value &params)
{
  std::string subscriptionId;
  std::string eventId = obj->getId() + "/" + eventType;
  std::shared_ptr <EventHandler> handler;
  std::unique_lock<std::mutex> lock (mutex);

  if (handlers.find (eventId) != handlers.end() ) {
    handler = handlers[eventId].lock();
  }

  if (!handler) {
    handler = std::shared_ptr <EventHandler> (new RabbitMQEventHandler (obj,
              getConnection()->getAddress(), getConnection()->getPort(),
              EVENT_EXCHANGE_PREFIX + pipelineId, eventId),
              std::bind (&RabbitMQPipeline::destroyHandler, this, std::placeholders::_1) );

    subscriptionId = processor->connectEventHandler (obj, sessionId, eventType,
                     handler);
    handlers[eventId] = std::weak_ptr <EventHandler> (handler);
  } else {
    subscriptionId = generateUUID();
    processor->registerEventHandler (obj, sessionId, subscriptionId, handler);
  }

  return subscriptionId;
}

void RabbitMQPipeline::destroyHandler (EventHandler *handler)
{
  RabbitMQEventHandler *rabbitHandler = dynamic_cast <RabbitMQEventHandler *>
                                        (handler);

  if (rabbitHandler != NULL) {
    std::string eventId;
    std::unique_lock<std::mutex> lock (mutex);

    eventId = rabbitHandler->getRoutingKey();

    if (handlers.find (eventId) != handlers.end() ) {
      handlers.erase (eventId);
    }
  }

  delete handler;
}

RabbitMQPipeline::StaticConstructor RabbitMQPipeline::staticConstructor;

RabbitMQPipeline::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
