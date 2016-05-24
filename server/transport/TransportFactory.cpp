/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gst/gst.h>
#include "TransportFactory.hpp"

#define GST_CAT_DEFAULT kurento_transport_factory
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoTransportFactory"

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
                                     (new WebSocketTransportFactory() ) );
}

} /* kurento */
