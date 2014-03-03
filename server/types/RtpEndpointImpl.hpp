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

#ifndef __RTP_ENDPOINT_IMPL_HPP__
#define __RTP_ENDPOINT_IMPL_HPP__

#include "SdpEndpointImpl.hpp"
#include <generated/RtpEndpoint.hpp>

namespace kurento
{

class RtpEndpointImpl : public virtual RtpEndpoint, public SdpEndpointImpl
{
public:
  RtpEndpointImpl (std::shared_ptr< MediaObjectImpl > mediaPipeline,
                   int garbagePeriod);
  virtual ~RtpEndpointImpl() {};
};

} /* kurento */

#endif /* __RTP_ENDPOINT_IMPL_HPP__ */
