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

#ifndef __MEDIA_PIPELINE_IMPL_HPP__
#define __MEDIA_PIPELINE_IMPL_HPP__

#include "MediaObjectImpl.hpp"
#include "jsoncpp/json/json.h"
#include <memory>

#include <generated/MediaPipeline.hpp>

namespace kurento
{

class MediaPipelineImpl : public virtual MediaPipeline, public MediaObjectImpl,
  public std::enable_shared_from_this<MediaPipeline>
{
public:
  MediaPipelineImpl (int garbageCollectorPeriod);
  virtual ~MediaPipelineImpl();

  bool getUnregChilds () {
    return false;
  }

  std::shared_ptr<MediaPipeline> getMediaPipeline() {
    return shared_from_this();
  }

  GstElement *getPipeline() {
    return pipeline;
  }

private:
  GstElement *pipeline;

  std::function <void (GstMessage *message) > busMessageLambda;
};

} /* kurento */

#endif /* __MEDIA_PIPELINE_IMPL_HPP__ */
