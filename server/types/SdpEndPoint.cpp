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
#include <gst/sdp/gstsdpmessage.h>

namespace kurento
{

SdpEndPoint::SdpEndPoint (std::shared_ptr<MediaObjectImpl> parent, SdpEndPointType::type type) : EndPoint (parent)
{
  this->type.__set_sdpEndPoint (type);
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
    MediaServerException e = MediaServerException ();
    e.__set_description ("Error creating SDP message");
    throw e;
  }

  result = gst_sdp_message_parse_buffer ( (const guint8 *) sdpStr.c_str (), -1, sdp);

  if (result != GST_SDP_OK) {
    gst_sdp_message_free (sdp);
    MediaServerException e = MediaServerException ();
    e.__set_description ("Error parsing SDP");
    throw e;
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
    MediaServerException e = MediaServerException ();
    e.__set_description ("No gst sdpendpoint element");
    throw e;
  }

  g_signal_emit_by_name (element, "generate-offer", &offer);

  if (offer == NULL) {
    MediaServerException e = MediaServerException ();
    e.__set_description ("Error generating offer");
    throw e;
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
    MediaServerException e = MediaServerException ();
    e.__set_description ("Error processing offer");
    throw e;
  }

  resultStr = sdp_to_str (result);
  gst_sdp_message_free (result);

  return resultStr;
}

std::string
SdpEndPoint::getLocalSessionDescription () throw (MediaServerException)
{
  GstSDPMessage *localSdp = NULL;
  std::string localSdpStr;

  g_object_get (element, "local-answer-sdp", &localSdp, NULL);

  if (localSdp == NULL) {
    g_object_get (element, "local-offer-sdp", &localSdp, NULL);
  }

  if (localSdp == NULL) {
    MediaServerException e = MediaServerException ();
    e.__set_description ("No local SDP");
    throw e;
  }

  localSdpStr = sdp_to_str (localSdp);
  gst_sdp_message_free (localSdp);

  return localSdpStr;
}

std::string
SdpEndPoint::getRemoteSessionDescription () throw (MediaServerException)
{
  GstSDPMessage *remoteSdp = NULL;
  std::string remoteSdpStr;

  g_object_get (element, "remote-answer-sdp", &remoteSdp, NULL);

  if (remoteSdp == NULL) {
    g_object_get (element, "remote-offer-sdp", &remoteSdp, NULL);
  }

  if (remoteSdp == NULL) {
    MediaServerException e = MediaServerException ();
    e.__set_description ("No remote SDP");
    throw e;
  }

  remoteSdpStr = sdp_to_str (remoteSdp);;
  gst_sdp_message_free (remoteSdp);

  return remoteSdpStr;
}

} // kurento
