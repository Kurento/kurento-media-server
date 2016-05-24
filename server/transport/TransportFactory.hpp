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

#ifndef __TRANSPORT_FACTORY_HPP__
#define __TRANSPORT_FACTORY_HPP__

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
