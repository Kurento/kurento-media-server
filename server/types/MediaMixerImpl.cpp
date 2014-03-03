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

#include "MediaMixerImpl.hpp"
#include "MediaPipelineImpl.hpp"

#define GST_CAT_DEFAULT kurento_media_mixer_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaMixerImpl"

namespace kurento
{

MediaMixerImpl::MediaMixerImpl (const std::string &factoryName,
                                std::shared_ptr<MediaObjectImpl> parent,
                                int garbagePeriod) :
  MediaObjectImpl (parent, garbagePeriod)
{
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );

  element = gst_element_factory_make (factoryName.c_str(), NULL);

  g_object_ref (element);
  gst_bin_add (GST_BIN ( pipe->getPipeline() ), element);
  gst_element_sync_state_with_parent (element);
}

MediaMixerImpl::~MediaMixerImpl()
{
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );

  gst_bin_remove (GST_BIN ( pipe->getPipeline() ), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

MediaMixerImpl::StaticConstructor MediaMixerImpl::staticConstructor;

MediaMixerImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
