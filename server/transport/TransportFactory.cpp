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
#include "TransportFactory.hpp"
#include <ThriftTransport.hpp>
#include <RabbitMQTransport.hpp>

#define GST_CAT_DEFAULT kurento_transport_factory
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoTransportFactory"

namespace kurento
{

std::shared_ptr<Transport> TransportFactory::create_transport (
  const boost::property_tree::ptree
  &config, std::shared_ptr<Processor> processor)
{
  boost::property_tree::ptree netConfig =
    config.get_child ("mediaServer.net");

  if (netConfig.find ("thrift") != netConfig.not_found() ) {
    return std::shared_ptr<Transport> (new ThriftTransport (config, processor) );
  } else if (netConfig.find ("rabbitmq") != netConfig.not_found() ) {
    return std::shared_ptr<Transport> ( new RabbitMQTransport (config, processor) );
  }

  throw boost::property_tree::ptree_error ("Network interface cannt be tarted");
}

TransportFactory::StaticConstructor TransportFactory::staticConstructor;

TransportFactory::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
