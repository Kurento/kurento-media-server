/*
 * MediaPipeline.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MediaPipeline.hpp"
#include "RtpEndPoint.hpp"
#include "WebRtcEndPoint.hpp"
#include "PlayerEndPoint.hpp"
#include "RecorderEndPoint.hpp"
#include "HttpEndPoint.hpp"
#include "MainMixer.hpp"
#include "ZBarFilter.hpp"

#include <glibmm.h>

#define GST_CAT_DEFAULT kurento_media_pipeline
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaPipeline"

namespace kurento
{

static void
receive_message (GstBus *bus, GstMessage *message, gpointer pipeline)
{
  switch (message->type) {
  case GST_MESSAGE_ERROR:
    GST_ERROR ("Error on bus: %P", message);
    // TODO: Check if further notification is needed
    break;
  default:
    break;
  }
}

MediaPipeline::MediaPipeline (std::shared_ptr<MediaHandler> mediaHandler) : MediaObjectImpl()
{
  this->type.__set_mediaObject (MediaObjectType::type::MEDIA_PIPELINE);
  this->mediaHandler = mediaHandler;
  pipeline = gst_pipeline_new (token.c_str () );
  g_object_set (G_OBJECT (pipeline), "async-handling", TRUE, NULL);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline) );
  gst_bus_add_signal_watch (bus);

  g_signal_connect (bus, "message", G_CALLBACK (receive_message), pipeline);

  g_object_unref (bus);
}

MediaPipeline::~MediaPipeline() throw()
{
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline) );
  gst_bus_remove_signal_watch (bus);
  g_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_object_unref (pipeline);
}

void
MediaPipeline::sendEvent (MediaEvent &event)
{
  this->mediaHandler->sendEvent (event);
}

std::shared_ptr<SdpEndPoint>
MediaPipeline::createSdpEndPoint (const SdpEndPointType::type type)
{
  switch (type) {
  case SdpEndPointType::type::RTP_END_POINT:
    return std::shared_ptr<SdpEndPoint> (new RtpEndPoint (shared_from_this() ) );
  case SdpEndPointType::type::WEBRTC_END_POINT:
    return std::shared_ptr<SdpEndPoint> (new WebRtcEndPoint (shared_from_this() ) );
  default:
    MediaServerException  e = MediaServerException();
    e.__set_description (std::string ("SdpEndPointType type does not exist.") );
    throw e;
  }
}

std::shared_ptr<SdpEndPoint>
MediaPipeline::createSdpEndPoint (const SdpEndPointType::type type, const std::string &sdp)
{
  switch (type) {
  case SdpEndPointType::type::RTP_END_POINT:
    return std::shared_ptr<SdpEndPoint> (new RtpEndPoint (shared_from_this(), sdp) );
  case SdpEndPointType::type::WEBRTC_END_POINT:
    return std::shared_ptr<SdpEndPoint> (new WebRtcEndPoint (shared_from_this(), sdp) );
  default:
    MediaServerException  e = MediaServerException();
    e.__set_description (std::string ("SdpEndPointType type does not exist.") );
    throw e;
  }
}

std::shared_ptr<UriEndPoint>
MediaPipeline::createUriEndPoint (const UriEndPointType::type type, const std::string &uri)
{
  switch (type) {
  case UriEndPointType::type::PLAYER_END_POINT:
    return std::shared_ptr<UriEndPoint> (new PlayerEndPoint (shared_from_this(), uri) );
  case UriEndPointType::type::RECORDER_END_POINT:
    return std::shared_ptr<UriEndPoint> (new RecorderEndPoint (shared_from_this(), uri) );
  default:
    MediaServerException  e = MediaServerException();
    e.__set_description (std::string ("UriEndPointType type does not exist.") );
    throw e;
  }
}

std::shared_ptr<HttpEndPoint>
MediaPipeline::createHttpEndPoint ()
{
  return std::shared_ptr<HttpEndPoint> (new HttpEndPoint (shared_from_this() ) );
}

std::shared_ptr<Mixer>
MediaPipeline::createMixer (const MixerType::type type)
{
  switch (type) {
  case MixerType::type::MAIN_MIXER:
    return std::shared_ptr<Mixer> (new MainMixer (shared_from_this() ) );
  default:
    MediaServerException  e = MediaServerException();
    e.__set_description (std::string ("Mixer type does not exist.") );
    throw e;
  }
}

std::shared_ptr<Filter>
MediaPipeline::createFilter (const FilterType::type type)
{
  switch (type) {
  case FilterType::type::ZBAR_FILTER:
    return std::shared_ptr<Filter> (new ZBarFilter (shared_from_this() ) );
  default:
    MediaServerException  e = MediaServerException();
    e.__set_description (std::string ("Filter type does not exist.") );
    throw e;
  }
}

MediaPipeline::StaticConstructor MediaPipeline::staticConstructor;

MediaPipeline::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
