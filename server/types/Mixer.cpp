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

#include "Mixer.hpp"

namespace kurento
{

Mixer::Mixer (std::shared_ptr<MediaPipeline> parent, const std::string &mixerType,
              const std::map<std::string, KmsMediaParam> &params)
  : MediaObjectImpl (parent, params),
    KmsMediaMixer()
{
  this->mixerType = mixerType;
  this->objectType.__set_mixer (*this);
}

Mixer::~Mixer() throw ()
{

}

std::shared_ptr<MixerEndPoint>
Mixer::createMixerEndPoint ()
{
  GST_WARNING ("TODO: implement");
  return NULL;
}

} // kurento
