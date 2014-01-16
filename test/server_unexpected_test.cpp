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

#include "server_test_base.hpp"
#include <boost/test/unit_test.hpp>

#include "KmsMediaServer_constants.h"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "KmsMediaUriEndPointType_constants.h"
#include "KmsMediaPlayerEndPointType_constants.h"
#include "KmsMediaZBarFilterType_constants.h"
#include "KmsMediaPointerDetectorFilterType_constants.h"
#include "KmsMediaRecorderEndPointType_constants.h"
#include "KmsMediaRtpEndPointType_constants.h"
#include "KmsMediaSdpEndPointType_constants.h"
#include "KmsMediaChromaFilterType_constants.h"
#include "KmsMediaFaceOverlayFilterType_constants.h"
#include "KmsMediaGStreamerFilterType_constants.h"
#include "KmsMediaHttpEndPointType_constants.h"
#include "KmsMediaHttpGetEndPointType_constants.h"

#include "utils/marshalling.hpp"
#include "utils/utils.hpp"
#include "HandlerTest.hpp"

#include <gst/gst.h>
#include <glibmm/timeval.h>
#include <exception>

#include "common/MediaSet.hpp"

#define GST_CAT_DEFAULT _server_unexpected_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_unexpected_test"

using namespace kurento;

static std::map<std::string, KmsMediaParam> emptyParams =
  std::map<std::string, KmsMediaParam> ();

class ClientHandler : public F
{
public:
  ClientHandler() : F(), handlerTest (new HandlerTest() ) {
    handlerTest->start ();
  };

  ~ClientHandler () {
    handlerTest->stop();
  }

protected:
  boost::shared_ptr<HandlerTest> handlerTest;

  void check_create_NULL_params ();
  void check_chroma_without_params ();
  void check_chroma_NULL_params ();
  void check_chroma_set_background_wrong_params ();
  void check_face_overlay_set_image_wrong_params ();
  void check_gstreamer_filter_without_params ();
  void check_gstreamer_filter_NULL_params ();
  void check_http_get_end_point_wrong_params ();
  void check_player_end_point_bad_uri ();
  void check_player_end_point_play_without_uri ();
  void check_player_and_filter_bad_config ();
  void check_pointer_detector_create_element_wrong_params ();
  void check_pointer_detector_send_empty_parameters ();
  void check_recorder_end_point_with_NULL_params ();
  void check_sdp_end_point_NULL_sdp_end_point ();
  void check_sdp_end_point_NULL_offer ();
};

void
ClientHandler::check_create_NULL_params ()
{
  std::map<std::string, KmsMediaParam> params;

  try {
    createKmsMediaUriEndPointConstructorParams (params, NULL);
    BOOST_FAIL ("Create a KmsMediaUriEndPointConstrustrorParam with NULL "
                "data must throw a Exception");
  } catch (std::exception &e) {
    BOOST_CHECK_EQUAL (e.what(), "basic_string::_S_construct null not valid");
  }
}

void
ClientHandler::check_chroma_without_params ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef chromaFilter = KmsMediaObjectRef();

  client->createMediaPipeline (mediaPipeline);

  try {
    client->createMediaElement (chromaFilter, mediaPipeline,
                                g_KmsMediaChromaFilterType_constants.TYPE_NAME);
    BOOST_FAIL ("Create a chromaFilter without constructor parameter must throw "
                "KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (
      g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR, e.errorCode);
  }


  client->release (mediaPipeline);
}

void
ClientHandler::check_chroma_NULL_params ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef chromaFilter = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  std::string backgroundUri = "/tmp/img.png";
  KmsMediaChromaConstructorParams chromaParams;
  KmsMediaChromaBackgroundImage backgroundImage;
  KmsMediaParam param;
  KmsMediaInvocationReturn ret;

  client->createMediaPipeline (mediaPipeline);

  createStructParam (param, chromaParams,
                     g_KmsMediaChromaFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  params[g_KmsMediaChromaFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE] =
    param;
  client->createMediaElementWithParams (chromaFilter,
                                        mediaPipeline,
                                        g_KmsMediaChromaFilterType_constants.TYPE_NAME,
                                        params);

  client->release (mediaPipeline);
}

void
ClientHandler::check_chroma_set_background_wrong_params()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef chromaFilter = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  std::string backgroundUri = "/tmp/img.png";
  KmsMediaChromaConstructorParams chromaParams;
  KmsMediaChromaBackgroundImage backgroundImage;
  KmsMediaPointerDetectorWindow wrongParams;
  KmsMediaParam param;
  KmsMediaInvocationReturn ret;

  client->createMediaPipeline (mediaPipeline);

  backgroundImage.uri = backgroundUri;
  chromaParams.backgroundImage = backgroundImage;
  chromaParams.calibrationArea.x = 200;
  chromaParams.calibrationArea.y = 150;
  chromaParams.calibrationArea.width = 50;
  chromaParams.calibrationArea.height = 50;
  createStructParam (param, chromaParams,
                     g_KmsMediaChromaFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  params[g_KmsMediaChromaFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE] =
    param;
  client->createMediaElementWithParams (chromaFilter,
                                        mediaPipeline,
                                        g_KmsMediaChromaFilterType_constants.TYPE_NAME,
                                        params);

  params.clear ();
  createStructParam (param, wrongParams,
                     g_KmsMediaChromaFilterType_constants.SET_BACKGROUND_PARAM_BACKGROUND_IMAGE);
  params[g_KmsMediaChromaFilterType_constants.SET_BACKGROUND_PARAM_BACKGROUND_IMAGE]
    = param;

  try {
    client->invoke (ret, chromaFilter,
                    g_KmsMediaChromaFilterType_constants.SET_BACKGROUND, params);
    BOOST_FAIL ("Send wrong params must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                       e.errorCode);
  }

  client->release (mediaPipeline);
}

void
ClientHandler::check_face_overlay_set_image_wrong_params ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef faceOverlay = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;
  std::string originalUri = "https://ci.kurento.com/video/fiwarecut.webm";
  std::string resultUri;
  KmsMediaFaceOverlayImage image;
  KmsMediaPointerDetectorWindow wrongParams;
  KmsMediaParam param;

  client->createMediaPipeline (mediaPipeline);
  createKmsMediaUriEndPointConstructorParams (params, originalUri);
  client->createMediaElement (faceOverlay, mediaPipeline,
                              g_KmsMediaFaceOverlayFilterType_constants.TYPE_NAME);

  //marshalling data
  createStructParam (param, image,
                     g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY_PARAM_IMAGE);
  params[g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY_PARAM_IMAGE]
    = param;

  client->invoke (ret, faceOverlay,
                  g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY,
                  params);

  //marshalling data
  createStructParam (param, wrongParams,
                     g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY_PARAM_IMAGE);
  params[g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY_PARAM_IMAGE]
    = param;

  try {
    client->invoke (ret, faceOverlay,
                    g_KmsMediaFaceOverlayFilterType_constants.SET_IMAGE_OVERLAY,
                    params);
    BOOST_FAIL ("Send wrong params must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                       e.errorCode);
  }


  client->release (mediaPipeline);
}

void
ClientHandler::check_gstreamer_filter_without_params()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef gstreamerFilter = KmsMediaObjectRef();

  client->createMediaPipeline (mediaPipeline);

  try {
    client->createMediaElement (gstreamerFilter,
                                mediaPipeline,
                                g_KmsMediaGStreamerFilterType_constants.TYPE_NAME);
    BOOST_FAIL ("Create a gstreamerFilter without constructor parameter must "
                "throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (
      g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR, e.errorCode);
  }

  client->release (mediaPipeline);
}

void
ClientHandler::check_gstreamer_filter_NULL_params()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef gstreamerFilter = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;
  KmsMediaParam param;
  std::string command;

  createStringParam (param, command);
  params[g_KmsMediaGStreamerFilterType_constants.CONSTRUCTOR_PARAM_GSTREAMER_COMMAND]
    = param;

  try {
    client->createMediaElementWithParams (gstreamerFilter,
                                          mediaPipeline,
                                          g_KmsMediaGStreamerFilterType_constants.TYPE_NAME,
                                          params);
    BOOST_FAIL ("Create a gstreamerFilter with a NULL parameter must throw "
                "KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND,
                       e.errorCode);
  }

  client->release (mediaPipeline);
}

void
ClientHandler::check_http_get_end_point_wrong_params ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef httpEp = KmsMediaObjectRef();
  KmsMediaHttpGetEndPointConstructorParams wrongParams;
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;
  KmsMediaParam param;
  std::string url;

  wrongParams.__isset.profileType = true;

  createStructParam (param, wrongParams,
                     g_KmsMediaHttpGetEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  params[g_KmsMediaHttpGetEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE]
    = param;

  client->createMediaPipeline (mediaPipeline);

  client->createMediaElementWithParams (httpEp, mediaPipeline,
                                        g_KmsMediaHttpGetEndPointType_constants.TYPE_NAME,
                                        params);
  client->release (mediaPipeline);
}

void
ClientHandler::check_player_end_point_bad_uri ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef playerEndPoint = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;
  std::string originalUri = "kasdoaskdsalmd jasd askd asd";
  std::string resultUri;
  std::string callbackToken;

  client->createMediaPipeline (mediaPipeline);
  createKmsMediaUriEndPointConstructorParams (params, originalUri);
  client->createMediaElementWithParams (playerEndPoint, mediaPipeline,
                                        g_KmsMediaPlayerEndPointType_constants.TYPE_NAME,
                                        params);

  client->invoke (ret, playerEndPoint,
                  g_KmsMediaUriEndPointType_constants.GET_URI, emptyParams);

  BOOST_REQUIRE_NO_THROW (unmarshalStringInvocationReturn (resultUri, ret) );
  BOOST_CHECK_EQUAL (0, originalUri.compare (resultUri) );

  params.clear();
  client->invoke (ret, playerEndPoint, g_KmsMediaUriEndPointType_constants.START,
                  params);

  client->invoke (ret, playerEndPoint, g_KmsMediaUriEndPointType_constants.STOP,
                  params);

  client->release (mediaPipeline);
}

void
ClientHandler::check_player_end_point_play_without_uri ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef playerEndPoint = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;

  client->createMediaPipeline (mediaPipeline);

  try {
    client->createMediaElement (playerEndPoint, mediaPipeline,
                                g_KmsMediaPlayerEndPointType_constants.TYPE_NAME);
    BOOST_FAIL ("Create a playerEndPoint without parameters must throw a "
                "KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (
      g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR, e.errorCode);
  }

  params.clear();

  try {
    client->invoke (ret, playerEndPoint, g_KmsMediaUriEndPointType_constants.START,
                    params);
    BOOST_FAIL ("Change the state of a non create element must throw a "
                "KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND,
                       e.errorCode);
  }

  client->release (mediaPipeline);
}

void
ClientHandler::check_player_and_filter_bad_config ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef zbarFilter = KmsMediaObjectRef();
  KmsMediaObjectRef playerEndPoint = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;
  std::string originalUri = "asdsadas asdsadsad";
  std::string resultUri;
  std::string callbackToken;

  client->createMediaPipeline (mediaPipeline);
  createKmsMediaUriEndPointConstructorParams (params, originalUri);
  client->createMediaElementWithParams (playerEndPoint, mediaPipeline,
                                        g_KmsMediaPlayerEndPointType_constants.TYPE_NAME,
                                        params);
  client->createMediaElement (zbarFilter, mediaPipeline,
                              g_KmsMediaZBarFilterType_constants.TYPE_NAME);
  client->connectElements (playerEndPoint, zbarFilter);

  params.clear();
  client->invoke (ret, playerEndPoint, g_KmsMediaUriEndPointType_constants.START,
                  params);

  sleep (2);

  client->invoke (ret, playerEndPoint, g_KmsMediaUriEndPointType_constants.STOP,
                  params);

  client->release (mediaPipeline);
}

void
ClientHandler::check_pointer_detector_create_element_wrong_params ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef playerEndPoint = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaParam param;
  KmsMediaPointerDetectorWindowSet windowSet;
  std::string name;

  gint32 numElements = 5;

  //create window layouts
  for (int i = 0; i < numElements ; i++) {
    KmsMediaPointerDetectorWindow aux;
    aux.topRightCornerX = i * 3;
    aux.topRightCornerY = i * 4;
    aux.width = i * 5;
    aux.height = i * 6;
    aux.id = "window" + std::to_string (i);
    windowSet.windows.insert (aux);
  }

  //marshalling data
  createStructParam (param, windowSet,
                     g_KmsMediaPointerDetectorFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  params[g_KmsMediaPointerDetectorFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE]
    = param;

  client->createMediaPipeline (mediaPipeline);

  try {
    client->createMediaElementWithParams (playerEndPoint, mediaPipeline,
                                          g_KmsMediaPlayerEndPointType_constants.TYPE_NAME,
                                          params);
    BOOST_FAIL ("Create an element with wrong media params must throw a "
                "KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (
      g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR, e.errorCode);
  }

  client->release (mediaPipeline);
}

void
ClientHandler::check_pointer_detector_send_empty_parameters ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef pointerDetectorFilter = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaParam param;
  KmsMediaPointerDetectorWindowSet windowSet;
  KmsMediaPointerDetectorWindow window;
  KmsMediaInvocationReturn ret;

  //marshalling data
  createStructParam (param, windowSet,
                     g_KmsMediaPointerDetectorFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  params[g_KmsMediaPointerDetectorFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE]
    = param;

  //create elements
  client->createMediaPipeline (mediaPipeline);
  client->createMediaElementWithParams (pointerDetectorFilter, mediaPipeline,
                                        g_KmsMediaPointerDetectorFilterType_constants.TYPE_NAME,
                                        params);

  params.clear();

  //add new Window
  //marshalling data
  createStructParam (param, window,
                     g_KmsMediaPointerDetectorFilterType_constants.ADD_NEW_WINDOW_PARAM_WINDOW);
  params[g_KmsMediaPointerDetectorFilterType_constants.ADD_NEW_WINDOW_PARAM_WINDOW]
    = param;

  client->invoke (ret, pointerDetectorFilter,
                  g_KmsMediaPointerDetectorFilterType_constants.ADD_NEW_WINDOW,
                  params);
  client->release (mediaPipeline);
}

void
ClientHandler::check_recorder_end_point_with_NULL_params ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef recorderEndPoint = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  const std::string uriSrc = "https://ci.kurento.com/video/small.webm";
  const std::string uriDst = "file:///tmp/test/player_end_point_test.webm";
  std::string resultUri;
  KmsMediaParam p;

  client->createMediaPipeline (mediaPipeline);
  createKmsMediaUriEndPointConstructorParams (params, uriDst);
  params[g_KmsMediaRecorderEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE] =
    p;

  try {
    client->createMediaElementWithParams (recorderEndPoint, mediaPipeline,
                                          g_KmsMediaRecorderEndPointType_constants.TYPE_NAME,
                                          params);
    BOOST_FAIL ("Create an element with non initialized media params must "
                "throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                       e.errorCode);
  }

  client->release (mediaPipeline);
}

void
ClientHandler::check_sdp_end_point_NULL_sdp_end_point ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef sdpEpA;

  std::map<std::string, KmsMediaParam> emptyParams =
    std::map<std::string, KmsMediaParam> ();
  KmsMediaInvocationReturn ret;

  try {
    client->invoke (ret, sdpEpA, g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP,
                    emptyParams);
    BOOST_FAIL ("Get local SDP from a NULL must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND,
                       e.errorCode);
  }
}

void
ClientHandler::check_sdp_end_point_NULL_offer ()
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef sdpEpA = KmsMediaObjectRef();
  KmsMediaInvocationReturn ret;

  std::map<std::string, KmsMediaParam> paramsProcessOffer;

  try {
    client->invoke (ret, sdpEpA,
                    g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER,
                    paramsProcessOffer);
    BOOST_FAIL ("Offer a NULL SDP must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND,
                       e.errorCode);
  }
}

BOOST_FIXTURE_TEST_SUITE ( server_unexpected_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_unexpected_test )
{
  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_create_NULL_params ();
  check_chroma_without_params ();
  check_chroma_NULL_params ();
  check_chroma_set_background_wrong_params();
  check_face_overlay_set_image_wrong_params ();
  check_gstreamer_filter_without_params ();
  check_gstreamer_filter_NULL_params ();
  check_http_get_end_point_wrong_params ();
  check_player_end_point_bad_uri ();
  check_player_end_point_play_without_uri ();
  check_player_and_filter_bad_config ();
  check_pointer_detector_create_element_wrong_params ();
  check_pointer_detector_send_empty_parameters ();
  check_recorder_end_point_with_NULL_params ();
  check_sdp_end_point_NULL_sdp_end_point ();
  check_sdp_end_point_NULL_offer ();
}

BOOST_AUTO_TEST_SUITE_END()
