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

#ifndef __MEDIA_PIPELINE_HPP__
#define __MEDIA_PIPELINE_HPP__

#include "MediaObjectImpl.hpp"
#include "MediaHandler.hpp"
#include <common/MediaSet.hpp>

namespace kurento
{

class SdpEndPoint;
class UriEndPoint;
class HttpEndPoint;
class Mixer;
class Filter;

class MediaPipeline : public MediaObjectImpl, public std::enable_shared_from_this<MediaPipeline>
{

public:
  MediaPipeline(std::shared_ptr<MediaHandler> mediaHandler);
  ~MediaPipeline() throw();

  std::shared_ptr<SdpEndPoint> createSdpEndPoint(const SdpEndPointType::type type);
  std::shared_ptr<SdpEndPoint> createSdpEndPoint(const SdpEndPointType::type type, const std::string& sdp);
  std::shared_ptr<UriEndPoint> createUriEndPoint (const UriEndPointType::type type, const std::string& uri);
  std::shared_ptr<HttpEndPoint> createHttpEndPoint ();
  std::shared_ptr<Mixer> createMixer (const MixerType::type type);
  std::shared_ptr<Filter> createFilter (const FilterType::type type);

  void sendEvent (MediaEvent &event);

  GstElement *pipeline;

private:
  std::shared_ptr<MediaHandler> mediaHandler;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __MEDIA_PIPELINE_HPP__ */
