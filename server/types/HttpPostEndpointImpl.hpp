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

#ifndef __HTTP_POST_ENDPOINT_IMPL_HPP__
#define __HTTP_POST_ENDPOINT_IMPL_HPP__

#include "HttpEndpointImpl.hpp"
#include <generated/HttpPostEndpoint.hpp>
#include <functional>

namespace kurento
{

class HttpPostEndpointImpl : public virtual HttpPostEndpoint,
  public HttpEndpointImpl
{
public:
  HttpPostEndpointImpl (bool useEncodedMedia, int disconnectionTimeout,
                        std::shared_ptr< MediaObjectImpl > mediaPipeline,
                        int garbagePeriod);
  virtual ~HttpPostEndpointImpl() {};
private:
  std::function<void() > eosLambda;
};

} /* kurento */

#endif /* __HTTP_POST_ENDPOINT_IMPL_HPP__ */
