/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
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

#include "MixerPort.hpp"
#include "Mixer.hpp"
#include "KmsMediaMixerPortType_constants.h"

#define FACTORY_NAME "mixerendpoint"

#define GST_CAT_DEFAULT kurento_mixer_port
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMixerPort"

namespace kurento
{
int
MixerPort::getHandlerId()
{
  return this->handlerId;
}

MixerPort::MixerPort (MediaSet &mediaSet, std::shared_ptr<Mixer> parent,
                      const std::map<std::string, KmsMediaParam> &params)
  : EndPoint (mediaSet, parent, g_KmsMediaMixerPortType_constants.TYPE_NAME,
              params, FACTORY_NAME)
{
  mixer = parent;
  g_signal_emit_by_name (mixer->getMixer(), "handle-port", element, &handlerId);
}

MixerPort::~MixerPort() throw ()
{
  g_signal_emit_by_name (mixer->getMixer(), "unhandle-port", handlerId);
}


MixerPort::StaticConstructor MixerPort::staticConstructor;

MixerPort::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
