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

#include "MixerPortImpl.hpp"

#define GST_CAT_DEFAULT kurento_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMixerPortImpl"

#define FACTORY_NAME "mixerendpoint"

namespace kurento
{

MixerPortImpl::MixerPortImpl (std::shared_ptr< MediaObjectImpl > parent,
                              int garbagePeriod) :
  MediaElementImpl (FACTORY_NAME, parent, garbagePeriod)
{
  mixer = std::dynamic_pointer_cast<MediaMixerImpl> (parent);
  g_signal_emit_by_name (mixer->getGstreamerElement(), "handle-port", element,
                         &handlerId);
}

MixerPortImpl::~MixerPortImpl()
{
  g_signal_emit_by_name (mixer->getGstreamerElement(), "unhandle-port", element,
                         &handlerId);
}


std::shared_ptr<MediaObject>
MixerPort::Factory::createObject (std::shared_ptr<MediaMixer> mediaMixer,
                                  int garbagePeriod)
{
  std::shared_ptr<MediaObject> object (new MixerPortImpl (
                                         std::dynamic_pointer_cast<MediaObjectImpl> (mediaMixer),
                                         garbagePeriod) );

  return object;
}

MixerPortImpl::StaticConstructor MixerPortImpl::staticConstructor;

MixerPortImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
