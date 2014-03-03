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

#ifndef __WEB_RTC_ENDPOINT_IMPL_HPP__
#define __WEB_RTC_ENDPOINT_IMPL_HPP__

#include "SdpEndpointImpl.hpp"
#include <generated/WebRtcEndpoint.hpp>

namespace kurento
{

class WebRtcEndpointImpl : public virtual WebRtcEndpoint, public SdpEndpointImpl
{
public:
  WebRtcEndpointImpl (std::shared_ptr< MediaObjectImpl > mediaPipeline,
                      int garbagePeriod);
  virtual ~WebRtcEndpointImpl() {};
};

} /* kurento */

#endif /* __WEB_RTC_ENDPOINT_IMPL_HPP__ */
