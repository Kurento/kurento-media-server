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

#include "SdpEndPoint.hpp"

#include "KmsMediaErrorCodes_constants.h"
#include "KmsMediaSdpEndPointType_constants.h"
#include <gst/sdp/gstsdpmessage.h>
#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

namespace kurento
{

SdpEndPoint::SdpEndPoint (std::shared_ptr<MediaObjectImpl> parent, const std::string type)
  : EndPoint (parent, type)
{
}

SdpEndPoint::~SdpEndPoint() throw ()
{

}

static GstSDPMessage *
str_to_sdp (const std::string &sdpStr)
{
  GstSDPResult result;
  GstSDPMessage *sdp = NULL;

  result = gst_sdp_message_new (&sdp);

  if (result != GST_SDP_OK) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.SDP_CREATE_ERROR,
        "Error creating SDP message");
  }

  result = gst_sdp_message_parse_buffer ( (const guint8 *) sdpStr.c_str (), -1, sdp);

  if (result != GST_SDP_OK) {
    gst_sdp_message_free (sdp);
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.SDP_PARSE_ERROR,
        "Error parsing SDP");
  }

  return sdp;
}

static std::string
sdp_to_str (const GstSDPMessage *sdp)
{
  std::string sdpStr;
  gchar *sdpGchar;

  sdpGchar = gst_sdp_message_as_text (sdp);
  sdpStr = std::string (sdpGchar);
  free (sdpGchar);

  return sdpStr;
}

std::string
SdpEndPoint::generateOffer ()
{
  GstSDPMessage *offer = NULL;
  std::string offerStr;

  if (element == NULL) {
  }

  g_signal_emit_by_name (element, "generate-offer", &offer);

  if (offer == NULL) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.SDP_END_POINT_GENERATE_OFFER_ERROR,
        "Error generating offer");
  }

  offerStr = sdp_to_str (offer);
  gst_sdp_message_free (offer);

  return offerStr;
}

std::string
SdpEndPoint::processAnswer (const std::string &answer)
{
  GstSDPMessage *answerSdp;
  std::string resultStr;

  answerSdp = str_to_sdp (answer);
  g_signal_emit_by_name (element, "process-answer", answerSdp, NULL);
  gst_sdp_message_free (answerSdp);

  return getLocalSessionDescription ();
}

std::string
SdpEndPoint::processOffer (const std::string &offer)
{
  GstSDPMessage *offerSdp = NULL, *result = NULL;
  std::string resultStr;

  offerSdp = str_to_sdp (offer);
  g_signal_emit_by_name (element, "process-offer", offerSdp, &result);
  gst_sdp_message_free (offerSdp);

  if (result == NULL) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.SDP_END_POINT_PROCESS_OFFER_ERROR,
        "Error processing offer");
  }

  resultStr = sdp_to_str (result);
  gst_sdp_message_free (result);

  return resultStr;
}

std::string
SdpEndPoint::getLocalSessionDescription () throw (KmsMediaServerException)
{
  GstSDPMessage *localSdp = NULL;
  std::string localSdpStr;

  g_object_get (element, "local-answer-sdp", &localSdp, NULL);

  if (localSdp == NULL) {
    g_object_get (element, "local-offer-sdp", &localSdp, NULL);
  }

  if (localSdp == NULL) {
    throw createKmsMediaServerException  (g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_LOCAL_SDP_ERROR,
        "No local SDP");
  }

  localSdpStr = sdp_to_str (localSdp);
  gst_sdp_message_free (localSdp);

  return localSdpStr;
}

std::string
SdpEndPoint::getRemoteSessionDescription () throw (KmsMediaServerException)
{
  GstSDPMessage *remoteSdp = NULL;
  std::string remoteSdpStr;

  g_object_get (element, "remote-answer-sdp", &remoteSdp, NULL);

  if (remoteSdp == NULL) {
    g_object_get (element, "remote-offer-sdp", &remoteSdp, NULL);
  }

  if (remoteSdp == NULL) {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_REMOTE_SDP_ERROR,
        "No remote SDP");
  }

  remoteSdpStr = sdp_to_str (remoteSdp);;
  gst_sdp_message_free (remoteSdp);

  return remoteSdpStr;
}

std::shared_ptr<KmsMediaInvocationReturn>
SdpEndPoint::invoke (const std::string &command, const std::map< std::string, KmsMediaParam >& params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP.compare (command) == 0) {
    return createStringInvocationReturn (getLocalSessionDescription () );
  } else if (g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP.compare (command) == 0) {
    return createStringInvocationReturn (getRemoteSessionDescription () );
  } else if (g_KmsMediaSdpEndPointType_constants.GENERATE_SDP_OFFER.compare (command) == 0) {
    return createStringInvocationReturn (generateOffer () );
  } else if (g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER.compare (command) == 0) {
    std::string offer = getStringParam (params, g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER_PARAM_OFFER_STR);
    return createStringInvocationReturn (processOffer (offer) );
  } else if (g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_ANSWER.compare (command) == 0) {
    std::string answer = getStringParam (params, g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_ANSWER_PARAM_ANSWER_STR);
    return createStringInvocationReturn (processAnswer (answer) );
  }

  return EndPoint::invoke (command, params);
}

} // kurento
