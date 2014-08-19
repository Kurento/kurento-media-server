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

#ifndef __WEBSOCKET_TRANSPORT_FACTORY_HPP__
#define __WEBSOCKET_TRANSPORT_FACTORY_HPP__

#include <TransportFactory.hpp>

namespace kurento
{

class WebSocketTransportFactory: public TransportFactory
{
public:
  WebSocketTransportFactory () {};
  virtual ~WebSocketTransportFactory() throw () {};

  virtual std::string getName () {
    return "websocket";
  }

  virtual std::shared_ptr<Transport> create (const boost::property_tree::ptree
      &config, std::shared_ptr<Processor> processor);

};

} /* kurento */

#endif /* __WEBSOCKET_TRANSPORT_FACTORY_HPP__ */
