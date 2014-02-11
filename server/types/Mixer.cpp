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

GstElement *
Mixer::getMixer()
{
  return this->element;
}

Mixer::Mixer (MediaSet &mediaSet,
              std::shared_ptr<MediaPipeline> parent,
              const std::string &mixerType,
              const std::map<std::string, KmsMediaParam> &params,
              const std::string &factoryName)
  : MediaObjectParent (mediaSet, parent, params),
    KmsMediaMixer()
{
  element = gst_element_factory_make (factoryName.c_str(), NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
  this->mixerType = mixerType;
  this->objectType.__set_mixer (*this);
}

Mixer::~Mixer() throw ()
{
  gst_bin_remove (GST_BIN (std::dynamic_pointer_cast<MediaPipeline>
                           (parent)->pipeline ), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

std::shared_ptr<MixerPort>
Mixer::createMixerPort ()
{
  std::shared_ptr<MixerPort> mixerPort (new MixerPort (getMediaSet(),
                                        shared_from_this(), emptyParams) );
  return mixerPort;
}

} // kurento
