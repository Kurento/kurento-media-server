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

#ifndef __TRANSPORT_FACTORY_HPP__
#define __TRANSPORT_FACTORY_HPP__

#include <glibmm.h>
#include "Transport.hpp"
#include "Processor.hpp"
#include <boost/property_tree/ptree.hpp>

namespace kurento
{

class TransportFactory
{
public:
  static std::shared_ptr<Transport> create_transport (const
      boost::property_tree::ptree &config, std::shared_ptr<Processor> processor);
  static void registerFactory (std::shared_ptr<TransportFactory> f);

  virtual std::string getName () = 0;
  virtual std::shared_ptr<Transport> create (const boost::property_tree::ptree
      &config, std::shared_ptr<Processor> processor) = 0;

private:

  static std::map<std::string, std::shared_ptr<TransportFactory>> factories;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __TRANSPORT_FACTORY_HPP__ */
