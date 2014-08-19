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

#include "RabbitMQTransportFactory.hpp"
#include "RabbitMQTransport.hpp"

namespace kurento
{

std::shared_ptr<Transport> RabbitMQTransportFactory::create (
  const boost::property_tree::ptree &config, std::shared_ptr<Processor> processor)
{
  return std::shared_ptr<Transport> (new RabbitMQTransport (config, processor) );
}

} /* kurento */
