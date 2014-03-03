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

#include "SessionEndpointImpl.hpp"

#define GST_CAT_DEFAULT kurento_session_endpoint_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoSessionEndpointImpl"

namespace kurento
{

SessionEndpointImpl::SessionEndpointImpl (const std::string &factoryName,
    std::shared_ptr< MediaObjectImpl > parent,
    int garbagePeriod) :
  EndpointImpl (factoryName, parent,
                garbagePeriod)
{
}

SessionEndpointImpl::StaticConstructor SessionEndpointImpl::staticConstructor;

SessionEndpointImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
