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

Mixer::Mixer (std::shared_ptr<MediaPipeline> parent, MixerType::type type) : MediaObjectImpl (parent)
{
  this->type.__set_mixerType (type);
}

Mixer::~Mixer() throw ()
{

}

std::shared_ptr<MixerEndPoint>
Mixer::createMixerEndPoint ()
{
  //TODO: complete
  return std::shared_ptr<MixerEndPoint> (new MixerEndPoint (shared_from_this() ) );
}

} // kurento
