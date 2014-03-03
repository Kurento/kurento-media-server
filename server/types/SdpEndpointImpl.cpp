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

#include "SdpEndpointImpl.hpp"
#include <KurentoException.hpp>
#include <gst/sdp/gstsdpmessage.h>

#define GST_CAT_DEFAULT kurento_session_endpoint_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoSdpEndpointImpl"

namespace kurento
{

static GstSDPMessage *
str_to_sdp (const std::string &sdpStr)
{
  GstSDPResult result;
  GstSDPMessage *sdp = NULL;

  result = gst_sdp_message_new (&sdp);

  if (result != GST_SDP_OK) {
    throw KurentoException ("Error creating SDP message");
  }

  result = gst_sdp_message_parse_buffer ( (const guint8 *) sdpStr.c_str (), -1,
                                          sdp);

  if (result != GST_SDP_OK) {

    gst_sdp_message_free (sdp);
    throw KurentoException ("Error parsing SDP");
  }

  return sdp;
}

static void
sdp_to_str (std::string &_return, const GstSDPMessage *sdp)
{
  std::string sdpStr;
  gchar *sdpGchar;

  sdpGchar = gst_sdp_message_as_text (sdp);
  _return.clear ();
  _return.append (sdpGchar);
  free (sdpGchar);
}

SdpEndpointImpl::SdpEndpointImpl (const std::string &factoryName,
                                  std::shared_ptr< MediaObjectImpl > parent,
                                  int garbagePeriod) :
  SessionEndpointImpl (factoryName, parent,
                       garbagePeriod)
{
//   TODO: Add support for this events
//   g_signal_connect (element, "media-start", G_CALLBACK (media_start_cb), this);
//   g_signal_connect (element, "media-stop", G_CALLBACK (media_stop_cb), this);
}

std::string
SdpEndpointImpl::generateOffer()
{
  GstSDPMessage *offer = NULL;
  std::string offerStr;

  if (element == NULL) {
  }

  g_signal_emit_by_name (element, "generate-offer", &offer);

  if (offer == NULL) {
    throw KurentoException ("Error generating offer");
  }

  sdp_to_str (offerStr, offer);
  gst_sdp_message_free (offer);

  return offerStr;
}

std::string
SdpEndpointImpl::getLocalSessionDescriptor()
{
  GstSDPMessage *localSdp = NULL;
  std::string localSdpStr;

  g_object_get (element, "local-answer-sdp", &localSdp, NULL);

  if (localSdp == NULL) {
    g_object_get (element, "local-offer-sdp", &localSdp, NULL);
  }

  if (localSdp == NULL) {
    throw KurentoException ("No local SDP");
  }

  sdp_to_str (localSdpStr, localSdp);
  gst_sdp_message_free (localSdp);

  return localSdpStr;
}

std::string
SdpEndpointImpl::getRemoteSessionDescriptor()
{
  GstSDPMessage *remoteSdp = NULL;
  std::string remoteSdpStr;

  g_object_get (element, "remote-answer-sdp", &remoteSdp, NULL);

  if (remoteSdp == NULL) {
    g_object_get (element, "remote-offer-sdp", &remoteSdp, NULL);
  }

  if (remoteSdp == NULL) {
    throw KurentoException ("No remote SDP");
  }

  sdp_to_str (remoteSdpStr, remoteSdp);;
  gst_sdp_message_free (remoteSdp);

  return remoteSdpStr;
}

std::string
SdpEndpointImpl::processAnswer (const std::string &answer)
{
  GstSDPMessage *answerSdp;
  std::string resultStr;

  answerSdp = str_to_sdp (answer);
  g_signal_emit_by_name (element, "process-answer", answerSdp, NULL);
  gst_sdp_message_free (answerSdp);

  MediaSessionStarted event (shared_from_this(), MediaSessionStarted::getName() );
  signalMediaSessionStarted (event);

  return getLocalSessionDescriptor ();
}

std::string
SdpEndpointImpl::processOffer (const std::string &offer)
{
  GstSDPMessage *offerSdp = NULL, *result = NULL;
  std::string offerSdpStr;

  offerSdp = str_to_sdp (offer);
  g_signal_emit_by_name (element, "process-offer", offerSdp, &result);
  gst_sdp_message_free (offerSdp);

  if (result == NULL) {
    throw KurentoException ("Error processing offer");
  }

  sdp_to_str (offerSdpStr, result);
  gst_sdp_message_free (result);

  MediaSessionStarted event (shared_from_this(), MediaSessionStarted::getName() );
  signalMediaSessionStarted (event);

  return offerSdpStr;
}

SdpEndpointImpl::StaticConstructor SdpEndpointImpl::staticConstructor;

SdpEndpointImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
