/*
 * RtpEndPoint.cpp - Kurento Media Server
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

#include "RtpEndPoint.hpp"
#include "media_config.hpp"

namespace kurento
{

void
RtpEndPoint::init (std::shared_ptr<MediaPipeline> parent)
{
  element = gst_element_factory_make ("rtpendpoint", NULL);

  g_object_set (element, "pattern-sdp", sdpPattern, NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
}

RtpEndPoint::RtpEndPoint (std::shared_ptr<MediaPipeline> parent)
  : SdpEndPoint (parent, SdpEndPointType::type::RTP_END_POINT)
{
  init (parent);
}

RtpEndPoint::RtpEndPoint (std::shared_ptr<MediaPipeline> parent, const std::string &sdp)
  : SdpEndPoint (parent, SdpEndPointType::type::RTP_END_POINT)
{
  init (parent);
  // TODO: use sdp
}

RtpEndPoint::~RtpEndPoint() throw ()
{
  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

} // kurento
