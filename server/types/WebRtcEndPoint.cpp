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

#include "WebRtcEndPoint.hpp"
#include "utils/utils.hpp"
#include "media_config.hpp"

#include "KmsMediaWebRtcEndPointType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT web_rtc_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoWebRtcEndPoint"

#define FACTORY_NAME "webrtcendpoint"

namespace kurento
{

WebRtcEndPoint::WebRtcEndPoint (MediaSet &mediaSet,
                                std::shared_ptr<MediaPipeline> parent,
                                const std::map<std::string, KmsMediaParam> &params)
  : SdpEndPoint (mediaSet, parent,
                 g_KmsMediaWebRtcEndPointType_constants.TYPE_NAME, params, FACTORY_NAME)
{
  g_object_set (element, "pattern-sdp", sdpPattern, NULL);

  //set properties
  GST_INFO ("stun port %d\n", stunServerPort);

  if (stunServerPort != 0) {
    g_object_set ( G_OBJECT (element), "stun-server-port", stunServerPort, NULL);
  }

  GST_INFO ("stun address %s\n", stunServerAddress.c_str() );
  g_object_set ( G_OBJECT (element), "stun-server", stunServerAddress.c_str(),
                 NULL);

  if (pemCertificate.compare ("") == 0) {
    GST_INFO ("Using default pemCertificate");
  } else {
    GST_INFO ("PemCertificate %s\n", pemCertificate.c_str() );
    g_object_set ( G_OBJECT (element), "certificate-pem-file",
                   pemCertificate.c_str(), NULL);
  }
}

WebRtcEndPoint::~WebRtcEndPoint() throw ()
{
}

WebRtcEndPoint::StaticConstructor WebRtcEndPoint::staticConstructor;

WebRtcEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
