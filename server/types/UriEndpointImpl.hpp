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

#ifndef __URI_ENDPOINT_IMPL_HPP__
#define __URI_ENDPOINT_IMPL_HPP__

#include "EndpointImpl.hpp"
#include <generated/UriEndpoint.hpp>

namespace kurento
{

class UriEndpointImpl: public virtual UriEndpoint, public EndpointImpl
{
public:
  UriEndpointImpl (const std::string &uri, const std::string &factoryName,
                   std::shared_ptr< MediaObjectImpl > parent,
                   int garbagePeriod);
  virtual ~UriEndpointImpl() throw () {};

  std::string getUri ();
  void pause ();
  void stop ();

protected:

  void start ();

private:

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __URI_ENDPOINT_IMPL_HPP__ */
