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

#include "DispatcherOneToManyImpl.hpp"
#include "MediaPipelineImpl.hpp"
#include "HubPortImpl.hpp"

#define GST_CAT_DEFAULT kurento_dispatcher_one_to_many_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoDispatcherOneToManyImpl"

#define FACTORY_NAME "dispatcheronetomany"
#define MAIN_END_POINT "main"
#define MAIN_PORT_DEFAULT (-1)

namespace kurento
{

DispatcherOneToManyImpl::DispatcherOneToManyImpl (
  std::shared_ptr<MediaObjectImpl> parent,
  int garbagePeriod) :
  HubImpl (FACTORY_NAME, parent, garbagePeriod)
{
}

void
DispatcherOneToManyImpl::setSource (std::shared_ptr<HubPort> source)
{
  std::shared_ptr<HubPortImpl> mixerPort =
    std::dynamic_pointer_cast<HubPortImpl> (source);
  int id = mixerPort->getHandlerId();

  g_object_set (G_OBJECT (element), MAIN_END_POINT, id, NULL);
}

void
DispatcherOneToManyImpl::removeSource ()
{
  g_object_set (G_OBJECT (element), MAIN_END_POINT, MAIN_PORT_DEFAULT, NULL);
}

std::shared_ptr<MediaObject>
DispatcherOneToMany::Factory::createObject (std::shared_ptr<MediaPipeline>
    parent,
    int garbagePeriod)
{
  std::shared_ptr <MediaObject> obj (new DispatcherOneToManyImpl (
                                       std::dynamic_pointer_cast<MediaObjectImpl> (parent), garbagePeriod) );

  return obj;
}

DispatcherOneToManyImpl::StaticConstructor
DispatcherOneToManyImpl::staticConstructor;

DispatcherOneToManyImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
