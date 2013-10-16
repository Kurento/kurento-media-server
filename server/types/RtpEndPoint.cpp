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

#include "RtpEndPoint.hpp"

#include "KmsMediaRtpEndPointType_constants.h"
#include "media_config.hpp"

#include "utils/utils.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

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

RtpEndPoint::RtpEndPoint (std::shared_ptr<MediaPipeline> parent, const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : SdpEndPoint (parent, g_KmsMediaRtpEndPointType_constants.TYPE_NAME, params)
{
  init (parent);
}

RtpEndPoint::~RtpEndPoint() throw ()
{
  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

} // kurento
