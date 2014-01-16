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

#include "utils/marshalling.hpp"
#include "utils/utils.hpp"
#include "HandlerTest.hpp"

#include <gst/gst.h>
#include <glibmm/timeval.h>

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

  void check_player_end_point_bad_uri ();
  void check_pointer_detector_send_empty_parameters ();
  void check_player_end_point_play_without_uri ();
  void check_player_and_filter_bad_config ();
  void check_pointer_detector_create_element_wrong_params ();
};

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
                                        g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);

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
                  g_KmsMediaPointerDetectorFilterType_constants.ADD_NEW_WINDOW, params);
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
    BOOST_FAIL ("Create a playerEndPoint without parameters must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (
      g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR, e.errorCode);
  }

  params.clear();

  try {
    client->invoke (ret, playerEndPoint, g_KmsMediaUriEndPointType_constants.START,
                    params);
    BOOST_FAIL ("Change the state of a non create element must throw a KmsMediaServerException");
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
                                        g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);
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
                                          g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);
    BOOST_FAIL ("Create an element with wrong media params must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (
      g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR, e.errorCode);
  }

  client->release (mediaPipeline);
}

BOOST_FIXTURE_TEST_SUITE ( server_unexpected_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_unexpected_test )
{
  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_player_end_point_bad_uri ();
  check_pointer_detector_send_empty_parameters ();
  check_player_end_point_play_without_uri ();
  check_player_and_filter_bad_config ();
  check_pointer_detector_create_element_wrong_params ();
}

BOOST_AUTO_TEST_SUITE_END()
