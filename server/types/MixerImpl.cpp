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

#include "MixerImpl.hpp"
#include "MediaPipelineImpl.hpp"
#include "HubPortImpl.hpp"

#define GST_CAT_DEFAULT kurento_mixer_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMixerImpl"

#define FACTORY_NAME "selectablemixer"

namespace kurento
{

MixerImpl::MixerImpl (
  std::shared_ptr<MediaObjectImpl> parent) :
  HubImpl (FACTORY_NAME, parent)
{
}

MediaObject *
Mixer::Factory::createObject (std::shared_ptr<MediaPipeline> parent)
{
  return new MixerImpl (
           std::dynamic_pointer_cast<MediaObjectImpl> (parent) );
}

void
MixerImpl::connect (MediaType media,
                    std::shared_ptr<HubPort> source,
                    std::shared_ptr<HubPort> sink)
{
  std::shared_ptr<HubPortImpl> sourcePort =
    std::dynamic_pointer_cast<HubPortImpl> (source);
  std::shared_ptr<HubPortImpl> sinkPort =
    std::dynamic_pointer_cast<HubPortImpl> (sink);
  std::string  action;
  bool connected;

  switch (media.getValue() ) {
  case MediaType::AUDIO:
    action = "connect-audio";
    break;

  case MediaType::VIDEO:
    action = "connect-video";
    break;

  default:
    throw KurentoException (UNSUPPORTED_MEDIA_TYPE, "Invalid media type");
  };

  g_signal_emit_by_name (G_OBJECT (element), action.c_str(),
                         sourcePort->getHandlerId(),
                         sinkPort->getHandlerId(),
                         &connected);

  if (!connected) {
    throw KurentoException (CONNECT_ERROR, "Can not connect video ports");
  }
}

void
MixerImpl::disconnect (MediaType media,
                       std::shared_ptr<HubPort> source,
                       std::shared_ptr<HubPort> sink)
{
  std::shared_ptr<HubPortImpl> sourcePort =
    std::dynamic_pointer_cast<HubPortImpl> (source);
  std::shared_ptr<HubPortImpl> sinkPort =
    std::dynamic_pointer_cast<HubPortImpl> (sink);
  std::string  action;
  bool connected;

  switch (media.getValue() ) {
  case MediaType::AUDIO:
    action = "disconnect-audio";
    break;

  default:
    /* Only audio is suppported so far */
    throw KurentoException (UNSUPPORTED_MEDIA_TYPE, "Invalid media type");
  };

  g_signal_emit_by_name (G_OBJECT (element), action.c_str(),
                         sourcePort->getHandlerId(),
                         sinkPort->getHandlerId(),
                         &connected);

  if (!connected) {
    throw KurentoException (CONNECT_ERROR, "Can not connect video ports");
  }
}

MixerImpl::StaticConstructor
MixerImpl::staticConstructor;

MixerImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
