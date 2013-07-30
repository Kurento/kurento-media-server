/*
 * MediaManager.cpp - Kurento Media Server
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

#include "MediaManager.hpp"
#include "RtpEndPoint.hpp"
#include "WebRtcEndPoint.hpp"
#include "PlayerEndPoint.hpp"
#include "RecorderEndPoint.hpp"
#include "HttpEndPoint.hpp"
#include "MainMixer.hpp"

#include <glibmm.h>

namespace kurento
{

MediaManager::MediaManager() : MediaObjectImpl()
{
}

MediaManager::~MediaManager() throw()
{
}

std::shared_ptr<SdpEndPoint>
MediaManager::createSdpEndPoint (const SdpEndPointType::type type)
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
MediaManager::createSdpEndPoint (const SdpEndPointType::type type, const std::string &sdp)
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
MediaManager::createUriEndpoint (const UriEndPointType::type type, const std::string &uri)
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
MediaManager::createHttpEndpoint ()
{
  return std::shared_ptr<HttpEndPoint> (new HttpEndPoint (shared_from_this() ) );
}

std::shared_ptr<Mixer>
MediaManager::createMixer (const MixerType::type type)
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

std::shared_ptr<Filter> createFilter (const FilterType::type type)
{
  // TODO: implement
  return NULL;
}

} // kurento
