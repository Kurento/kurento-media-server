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
  /* TODO */
}

void
MixerImpl::disconnect (MediaType media,
                       std::shared_ptr<HubPort> source,
                       std::shared_ptr<HubPort> sink)
{
  /* TODO */
}

MixerImpl::StaticConstructor
MixerImpl::staticConstructor;

MixerImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
