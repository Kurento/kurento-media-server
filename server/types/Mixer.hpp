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

#ifndef __MIXER_HPP__
#define __MIXER_HPP__

#include "MediaPipeline.hpp"
#include "MixerEndPoint.hpp"

namespace kurento
{

class Mixer : public MediaObjectImpl, public std::enable_shared_from_this<Mixer>
{
public:
  Mixer (std::shared_ptr<MediaPipeline> parent, MixerType::type type);
  ~Mixer() throw ();

  std::shared_ptr<MixerEndPoint> createMixerEndPoint ();
};

} // kurento

#endif /* __MIXER_HPP__ */
