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

#define FACTORY_NAME "rtpendpoint"

namespace kurento
{

void
RtpEndPoint::init (std::shared_ptr<MediaPipeline> parent)
{
  g_object_set (element, "pattern-sdp", sdpPattern, NULL);
}

RtpEndPoint::RtpEndPoint (MediaSet &mediaSet,
                          std::shared_ptr<MediaPipeline> parent,
                          const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : SdpEndPoint (mediaSet, parent,
                 g_KmsMediaRtpEndPointType_constants.TYPE_NAME, params, FACTORY_NAME)
{
  init (parent);
}

RtpEndPoint::~RtpEndPoint() throw ()
{
}

} // kurento
