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

#include "DispatcherMixerImpl.hpp"
#include "MediaPipelineImpl.hpp"

#define GST_CAT_DEFAULT kurento_dispatcher_mixer_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoDispatcherMixerImpl"

#define FACTORY_NAME "mainmixer"

namespace kurento
{

DispatcherMixerImpl::DispatcherMixerImpl (
  std::shared_ptr<MediaObjectImpl> parent,
  int garbagePeriod) :
  MediaMixerImpl (FACTORY_NAME, parent, garbagePeriod)
{
  // TODO:
}

std::shared_ptr<MediaObject>
DispatcherMixer::Factory::createObject (std::shared_ptr<MediaPipeline> parent,
                                        int garbagePeriod)
{
  std::shared_ptr <MediaObject> obj (new DispatcherMixerImpl (
                                       std::dynamic_pointer_cast<MediaObjectImpl> (parent), garbagePeriod) );

  return obj;
}

DispatcherMixerImpl::StaticConstructor DispatcherMixerImpl::staticConstructor;

DispatcherMixerImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
