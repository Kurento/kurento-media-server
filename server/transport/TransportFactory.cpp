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

#define GST_CAT_DEFAULT kurento_transport_factory
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoTransportFactory"

#include <RabbitMQTransportFactory.hpp>
#include <WebSocketTransportFactory.hpp>

namespace kurento
{

std::shared_ptr<Transport> TransportFactory::create_transport (
  const boost::property_tree::ptree
  &config, std::shared_ptr<Processor> processor)
{
  boost::property_tree::ptree netConfig =
    config.get_child ("mediaServer.net");

  if (netConfig.size() > 1) {
    throw boost::property_tree::ptree_error ("Only one network interface can be configured");
  } else if (netConfig.size() == 0) {
    throw boost::property_tree::ptree_error ("No network interface is configured");
  }

  try {
    return factories.at (netConfig.begin()->first)->create (config, processor);
  } catch (std::out_of_range &e) {
    throw boost::property_tree::ptree_error ("Configured network interface has not been registered");
  }
}

void TransportFactory::registerFactory (std::shared_ptr<TransportFactory> f)
{
  factories[f->getName()] = f;
}

std::map<std::string, std::shared_ptr<TransportFactory>>
    TransportFactory::factories;

TransportFactory::StaticConstructor TransportFactory::staticConstructor;

TransportFactory::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
  TransportFactory::registerFactory (std::shared_ptr<TransportFactory>
                                     (new RabbitMQTransportFactory() ) );
  TransportFactory::registerFactory (std::shared_ptr<TransportFactory>
                                     (new WebSocketTransportFactory() ) );
}

} /* kurento */
