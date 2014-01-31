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
#include "KmsMediaSessionEndPointType_constants.h"
#include <gst/sdp/gstsdpmessage.h>
#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#define GST_CAT_DEFAULT kurento_sdp_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoSdpEndPoint"

namespace kurento
{

void
send_event (SdpEndPoint *self, guint type, bool local,
            const std::string &eventType)
{
  KmsMediaTransmissionData mdata;
  KmsMediaEventData eventData;
  bool end_session = false;

  switch (type) {
  case KmsMediaType::type::AUDIO:
    mdata.__set_media (KmsMediaType::type::AUDIO);
    break;

  case KmsMediaType::type::VIDEO:
    mdata.__set_media (KmsMediaType::type::VIDEO);
    break;

  default:
    GST_WARNING ("Unsupported media type %u", type);
    return;
  }

  if (local) {
    mdata.__set_source (KmsMediaSource::type::LOCAL);
  } else {
    mdata.__set_source (KmsMediaSource::type::REMOTE);
  }

  /* marshalling data */
  createStructParam (eventData, mdata,
                     g_KmsMediaSdpEndPointType_constants.EVENT_MEDIA_TRANSMISSION_DATA_TYPE);

  self->sendEvent (eventType, eventData);

  self->mutex.lock();

  if (eventType ==
      g_KmsMediaSdpEndPointType_constants.EVENT_MEDIA_TRANSMISSION_START) {
    self->ssrcs++;
  } else {
    self->ssrcs--;
  }

  end_session = (self->ssrcs <= 0);
  self->mutex.unlock();

  if (end_session) {
    self->sendEvent (
      g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_COMPLETE);
  }
}

static void
media_start_cb (GstElement *element, guint type, bool local,
                SdpEndPoint *self)
{
  send_event (self, type, local,
              g_KmsMediaSdpEndPointType_constants.EVENT_MEDIA_TRANSMISSION_START);
}

static void
media_stop_cb (GstElement *element, guint type, bool local,
               SdpEndPoint *self)
{
  send_event (self, type, local,
              g_KmsMediaSdpEndPointType_constants.EVENT_MEDIA_TRANSMISSION_STOP);
}

SdpEndPoint::SdpEndPoint (MediaSet &mediaSet,
                          std::shared_ptr<MediaObjectImpl> parent,
                          const std::string &type,
                          const std::map<std::string, KmsMediaParam> &params,
                          const std::string &factoryName)
  : EndPoint (mediaSet, parent, type, params, factoryName)
{
  ssrcs = 0;
  g_signal_connect (element, "media-start", G_CALLBACK (media_start_cb), this);
  g_signal_connect (element, "media-stop", G_CALLBACK (media_stop_cb), this);
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
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.SDP_CREATE_ERROR,
                                   "Error creating SDP message");
    throw except;
  }

  result = gst_sdp_message_parse_buffer ( (const guint8 *) sdpStr.c_str (), -1,
                                          sdp);

  if (result != GST_SDP_OK) {
    KmsMediaServerException except;

    gst_sdp_message_free (sdp);
    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.SDP_PARSE_ERROR,
                                   "Error parsing SDP");
    throw except;
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

void
SdpEndPoint::generateOffer (std::string &_return)
{
  GstSDPMessage *offer = NULL;

  if (element == NULL) {
  }

  g_signal_emit_by_name (element, "generate-offer", &offer);

  if (offer == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.SDP_END_POINT_GENERATE_OFFER_ERROR,
                                   "Error generating offer");
    throw except;
  }

  sdp_to_str (_return, offer);
  gst_sdp_message_free (offer);
}

void
SdpEndPoint::processAnswer (std::string &_return, const std::string &answer)
{
  GstSDPMessage *answerSdp;
  std::string resultStr;

  answerSdp = str_to_sdp (answer);
  g_signal_emit_by_name (element, "process-answer", answerSdp, NULL);
  gst_sdp_message_free (answerSdp);

  getLocalSessionDescription (_return);

  sendEvent (
    g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_START);
}

void
SdpEndPoint::processOffer (std::string &_return, const std::string &offer)
{
  GstSDPMessage *offerSdp = NULL, *result = NULL;

  offerSdp = str_to_sdp (offer);
  g_signal_emit_by_name (element, "process-offer", offerSdp, &result);
  gst_sdp_message_free (offerSdp);

  if (result == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.SDP_END_POINT_PROCESS_OFFER_ERROR,
                                   "Error processing offer");
    throw except;
  }

  sdp_to_str (_return, result);
  gst_sdp_message_free (result);

  sendEvent (g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_START);
}

void
SdpEndPoint::getLocalSessionDescription (std::string &_return) throw (
  KmsMediaServerException)
{
  GstSDPMessage *localSdp = NULL;

  g_object_get (element, "local-answer-sdp", &localSdp, NULL);

  if (localSdp == NULL) {
    g_object_get (element, "local-offer-sdp", &localSdp, NULL);
  }

  if (localSdp == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException  (except,
                                    g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_LOCAL_SDP_ERROR,
                                    "No local SDP");
    throw except;
  }

  sdp_to_str (_return, localSdp);
  gst_sdp_message_free (localSdp);
}

void
SdpEndPoint::getRemoteSessionDescription (std::string &_return) throw (
  KmsMediaServerException)
{
  GstSDPMessage *remoteSdp = NULL;

  g_object_get (element, "remote-answer-sdp", &remoteSdp, NULL);

  if (remoteSdp == NULL) {
    g_object_get (element, "remote-offer-sdp", &remoteSdp, NULL);
  }

  if (remoteSdp == NULL) {
    KmsMediaServerException except;
    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_REMOTE_SDP_ERROR,
                                   "No remote SDP");
    throw except;
  }

  sdp_to_str (_return, remoteSdp);;
  gst_sdp_message_free (remoteSdp);
}

void
SdpEndPoint::invoke (KmsMediaInvocationReturn &_return,
                     const std::string &command,
                     const std::map< std::string, KmsMediaParam > &params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP.compare (command) == 0) {
    std::string localSdp;

    getLocalSessionDescription (localSdp);
    createStringInvocationReturn (_return, localSdp);
  } else if (g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP.compare (
               command) == 0) {
    std::string remoteSdp;

    getRemoteSessionDescription (remoteSdp);
    createStringInvocationReturn (_return, remoteSdp);
  } else if (g_KmsMediaSdpEndPointType_constants.GENERATE_SDP_OFFER.compare (
               command) == 0) {
    std::string offer;

    generateOffer (offer);
    createStringInvocationReturn (_return, offer);
  } else if (g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER.compare (
               command) == 0) {
    std::string offer;
    std::string answer;

    getStringParam (offer, params,
                    g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER_PARAM_OFFER_STR);
    processOffer (answer, offer);
    createStringInvocationReturn (_return, answer);
  } else if (g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_ANSWER.compare (
               command) == 0) {
    std::string answer;
    std::string localSdp;

    getStringParam (answer, params,
                    g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_ANSWER_PARAM_ANSWER_STR);
    processAnswer (localSdp, answer);
    createStringInvocationReturn (_return, localSdp);
  } else {
    EndPoint::invoke (_return, command, params);
  }
}

void
SdpEndPoint::subscribe (std::string &_return, const std::string &eventType,
                        const std::string &handlerAddress,
                        const int32_t handlerPort)
throw (KmsMediaServerException)
{
  if (g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_START ==
      eventType) {
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  } else if (g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_COMPLETE
             ==
             eventType) {
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  } else if (g_KmsMediaSdpEndPointType_constants.EVENT_MEDIA_TRANSMISSION_START
             ==
             eventType) {
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  } else if (g_KmsMediaSdpEndPointType_constants.EVENT_MEDIA_TRANSMISSION_STOP
             ==
             eventType) {
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  } else {
    EndPoint::subscribe (_return, eventType, handlerAddress, handlerPort);
  }
}

SdpEndPoint::StaticConstructor SdpEndPoint::staticConstructor;

SdpEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
