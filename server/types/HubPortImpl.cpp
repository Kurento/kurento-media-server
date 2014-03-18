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

#include "HubPortImpl.hpp"

#define GST_CAT_DEFAULT kurento_hub_port_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHubPortImpl"

#define FACTORY_NAME "mixerport"

namespace kurento
{

HubPortImpl::HubPortImpl (std::shared_ptr< MediaObjectImpl > parent,
                          int garbagePeriod) :
  MediaElementImpl (FACTORY_NAME, parent, garbagePeriod)
{
  mixer = std::dynamic_pointer_cast<HubImpl> (parent);
  g_signal_emit_by_name (mixer->getGstreamerElement(), "handle-port", element,
                         &handlerId);
}

HubPortImpl::~HubPortImpl()
{
  g_signal_emit_by_name (mixer->getGstreamerElement(), "unhandle-port",
                         handlerId);
}


MediaObject *
HubPort::Factory::createObject (std::shared_ptr<Hub> hub,
                                int garbagePeriod)
{
  return new HubPortImpl (std::dynamic_pointer_cast<MediaObjectImpl> (hub),
                          garbagePeriod);
}

HubPortImpl::StaticConstructor HubPortImpl::staticConstructor;

HubPortImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
