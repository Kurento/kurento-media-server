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

#include <KurentoException.hpp>

#include "DispatcherImpl.hpp"
#include "MediaPipelineImpl.hpp"
#include "HubPortImpl.hpp"

#define GST_CAT_DEFAULT kurento_dispatcher_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoDispatcherImpl"

#define FACTORY_NAME "dispatcher"

namespace kurento
{

DispatcherImpl::DispatcherImpl (
  std::shared_ptr<MediaObjectImpl> parent) :
  HubImpl (FACTORY_NAME, parent)
{
}

MediaObject *
Dispatcher::Factory::createObject (std::shared_ptr<MediaPipeline> parent)
{
  return new DispatcherImpl (
           std::dynamic_pointer_cast<MediaObjectImpl> (parent) );
}

void
DispatcherImpl::connect (std::shared_ptr<HubPort> source,
                         std::shared_ptr<HubPort> sink)
{
  std::shared_ptr<HubPortImpl> sourcePort =
    std::dynamic_pointer_cast<HubPortImpl> (source);
  std::shared_ptr<HubPortImpl> sinkPort =
    std::dynamic_pointer_cast<HubPortImpl> (sink);
  bool connected;

  g_signal_emit_by_name (G_OBJECT (element), "connect",
                         sourcePort->getHandlerId(),
                         sinkPort->getHandlerId(),
                         &connected);

  if (!connected) {
    throw KurentoException ("Can not connect ports");
  }
}


DispatcherImpl::StaticConstructor
DispatcherImpl::staticConstructor;

DispatcherImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
