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
#include "KmsMediaRecorderEndPointType_constants.h"
#include "KmsMediaHttpEndPointType_constants.h"
#include "KmsMediaZBarFilterType_constants.h"
#include "KmsMediaJackVaderFilterType_constants.h"

#include "utils/marshalling.hpp"

#include <gst/gst.h>

#include "common/MediaSet.hpp"

#define GST_CAT_DEFAULT _server_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_test"

using namespace kurento;

static std::map<std::string, KmsMediaParam> emptyParams = std::map<std::string, KmsMediaParam> ();

BOOST_FIXTURE_TEST_SUITE ( server_test_suite,  F)

static void
check_version (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  int32_t gotVersion;

  gotVersion = client->getVersion();
  BOOST_CHECK_EQUAL (gotVersion, g_KmsMediaServer_constants.VERSION);
}

#if 0 /* Temporally disabled */
static void
check_type (boost::shared_ptr<kurento::MediaServerServiceClient> client)
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId mo = MediaObjectId();

  client->createMediaPipeline (mediaPipeline, 0);
  BOOST_CHECK (mediaPipeline.type.__isset.mediaObject);
  BOOST_CHECK_EQUAL (mediaPipeline.type.mediaObject, MediaObjectType::type::MEDIA_PIPELINE);

  client->createSdpEndPoint (mo, mediaPipeline, SdpEndPointType::type::RTP_END_POINT);
  BOOST_CHECK (mo.type.__isset.sdpEndPoint);
  BOOST_CHECK_EQUAL (mo.type.sdpEndPoint, SdpEndPointType::type::RTP_END_POINT);

  client->createSdpEndPoint (mo, mediaPipeline, SdpEndPointType::type::WEBRTC_END_POINT);
  BOOST_CHECK (mo.type.__isset.sdpEndPoint);
  BOOST_CHECK_EQUAL (mo.type.sdpEndPoint, SdpEndPointType::type::WEBRTC_END_POINT);

  client->createUriEndPoint (mo, mediaPipeline, UriEndPointType::type::PLAYER_END_POINT, "");
  BOOST_CHECK (mo.type.__isset.uriEndPoint);
  BOOST_CHECK_EQUAL (mo.type.uriEndPoint, UriEndPointType::type::PLAYER_END_POINT);

  client->createUriEndPoint (mo, mediaPipeline, UriEndPointType::type::RECORDER_END_POINT, "");
  BOOST_CHECK (mo.type.__isset.uriEndPoint);
  BOOST_CHECK_EQUAL (mo.type.uriEndPoint, UriEndPointType::type::RECORDER_END_POINT);

  client->createHttpEndPoint (mo, mediaPipeline);
  BOOST_CHECK (mo.type.__isset.endPoint);
  BOOST_CHECK_EQUAL (mo.type.endPoint, EndPointType::type::HTTP_END_POINT);

  client->createMixer (mo, mediaPipeline, MixerType::type::MAIN_MIXER);
  BOOST_CHECK (mo.type.__isset.mixerType);
  BOOST_CHECK_EQUAL (mo.type.mixerType, MixerType::type::MAIN_MIXER);

  client->release (mediaPipeline);
}
#endif

static void
check_use_released_media_pipeline (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef mo = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;

  client->createMediaPipeline (mediaPipeline);
  client->release (mediaPipeline);
  params = createKmsMediaUriEndPointConstructorParams ("file:///tmp/f.webm");

  try {
    client->createMediaElementWithParams (mo, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);
    BOOST_FAIL ("Use a released MediaPipeline must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND, e.errorCode);
  }
}

static void
check_auto_released_media_pipeline (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef moA = KmsMediaObjectRef();
  KmsMediaObjectRef moB = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;

  params = createKmsMediaUriEndPointConstructorParams ("file:///tmp/f.webm");

  client->createMediaPipeline (mediaPipeline);
  g_usleep ( (2 * AUTO_RELEASE_INTERVAL + 1) * G_USEC_PER_SEC);

  try {
    client->keepAlive (mediaPipeline);
    BOOST_FAIL ("Use an auto released MediaPipeline must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND, e.errorCode);
  }

  client->createMediaPipeline (mediaPipeline);
  g_usleep (AUTO_RELEASE_INTERVAL * G_USEC_PER_SEC);
  BOOST_REQUIRE_NO_THROW (client->createMediaElementWithParams (moA, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params););

  g_usleep (AUTO_RELEASE_INTERVAL * G_USEC_PER_SEC);
  BOOST_REQUIRE_NO_THROW (client->keepAlive (mediaPipeline) );
  BOOST_REQUIRE_NO_THROW (client->keepAlive (moA) );

  g_usleep (AUTO_RELEASE_INTERVAL * G_USEC_PER_SEC);
  BOOST_REQUIRE_NO_THROW (client->createMediaElementWithParams (moB, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params););

  g_usleep ( AUTO_RELEASE_INTERVAL * G_USEC_PER_SEC);
  BOOST_REQUIRE_NO_THROW (client->keepAlive (moB) );
  g_usleep ( AUTO_RELEASE_INTERVAL * G_USEC_PER_SEC);

  try {
    client->keepAlive (moA);
    BOOST_FAIL ("Use an auto released MediaObject must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND, e.errorCode);
  }

  BOOST_REQUIRE_NO_THROW (client->keepAlive (moB) );
  BOOST_REQUIRE_NO_THROW (client->keepAlive (mediaPipeline) );

  g_usleep ( (2 * AUTO_RELEASE_INTERVAL + 1) * G_USEC_PER_SEC);

  try {
    client->keepAlive (moB);
    BOOST_FAIL ("Use an auto released MediaObject must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND, e.errorCode);
  }

  try {
    client->keepAlive (mediaPipeline);
    BOOST_FAIL ("Use an auto released MediaPipeline must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_FOUND, e.errorCode);
  }
}

static void
check_keep_alive_media_pipeline (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  int i;
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();

  client->createMediaPipeline (mediaPipeline);

  for (i = 0; i < 5; i++) {
    g_usleep (AUTO_RELEASE_INTERVAL * G_USEC_PER_SEC);
    client->keepAlive (mediaPipeline);
  }

  client->release (mediaPipeline);
}

static void
check_exclude_from_gc (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;

  params = createKmsMediaObjectConstructorParams (true);

  client->createMediaPipelineWithParams (mediaPipeline, params);
  g_usleep ( (2 * AUTO_RELEASE_INTERVAL + 1) * G_USEC_PER_SEC);
  BOOST_REQUIRE_NO_THROW (client->keepAlive (mediaPipeline) );
  BOOST_REQUIRE_NO_THROW (client->release (mediaPipeline) );
}

static void
check_parent (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef mo = KmsMediaObjectRef();
  KmsMediaObjectRef parent = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;

  client->createMediaPipeline (mediaPipeline);
  params = createKmsMediaUriEndPointConstructorParams ("file:///tmp/f.webm");
  client->createMediaElementWithParams (mo, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);
  client->getParent (parent, mo);
  BOOST_CHECK_EQUAL (mediaPipeline.id, parent.id);

  client->release (mediaPipeline);
}

static void
check_get_parent_of_media_pipeline (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef parent = KmsMediaObjectRef();

  client->createMediaPipeline (mediaPipeline);

  try {
    client->getParent (parent, mediaPipeline);
    BOOST_FAIL ("Get parent of a MediaPipeline must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_HAS_NOT_PARENT, e.errorCode);
  }

  client->release (mediaPipeline);
}

static void
check_getMediaPipeline (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef mo = KmsMediaObjectRef();
  KmsMediaObjectRef mediaPipelineGot = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;

  client->createMediaPipeline (mediaPipeline);
  client->getMediaPipeline (mediaPipelineGot, mediaPipeline);
  BOOST_CHECK_EQUAL (mediaPipeline.id, mediaPipelineGot.id);

  params = createKmsMediaUriEndPointConstructorParams ("file:///tmp/f.webm");
  client->createMediaElementWithParams (mo, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);
  client->getMediaPipeline (mediaPipelineGot, mo);
  BOOST_CHECK_EQUAL (mediaPipeline.id, mediaPipelineGot.id);

  client->release (mediaPipeline);
}

static void
check_same_token (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef mo = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;

  client->createMediaPipeline (mediaPipeline);
  params = createKmsMediaUriEndPointConstructorParams ("file:///tmp/f.webm");
  client->createMediaElementWithParams (mo, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);
  BOOST_CHECK_EQUAL (mediaPipeline.token, mo.token);

  client->release (mediaPipeline);
}

static void
check_get_media_element_from_pad (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef playerEndPoint = KmsMediaObjectRef();
  KmsMediaObjectRef me = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  std::vector<KmsMediaObjectRef> pads;
  std::vector<KmsMediaObjectRef>::iterator it;

  client->createMediaPipeline (mediaPipeline);
  params = createKmsMediaUriEndPointConstructorParams ("file:///tmp/f.webm");
  client->createMediaElementWithParams (playerEndPoint, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);

  client->getMediaSrcs (pads, playerEndPoint);

  for (it = pads.begin(); it != pads.end(); ++it) {
    client->getMediaElement (me, *it);
    BOOST_CHECK_EQUAL (playerEndPoint.id, me.id);
  }

  client->getMediaSinks (pads, playerEndPoint);

  for (it = pads.begin(); it != pads.end(); ++it) {
    client->getMediaElement (me, *it);
    BOOST_CHECK_EQUAL (playerEndPoint.id, me.id);
  }

  client->release (mediaPipeline);
}

static void
check_player_end_point (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef playerEndPoint = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;
  std::string originalUri = "file:///tmp/player_end_point_test.webm";
  std::string resultUri;
  std::string callbackToken;

  client->createMediaPipeline (mediaPipeline);
  params = createKmsMediaUriEndPointConstructorParams (originalUri);
  client->createMediaElementWithParams (playerEndPoint, mediaPipeline, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params);

  client->invoke (ret, playerEndPoint, g_KmsMediaUriEndPointType_constants.GET_URI, emptyParams);

  BOOST_REQUIRE_NO_THROW (resultUri = unmarshalStringInvocationReturn (ret) );
  BOOST_CHECK_EQUAL (0, originalUri.compare (resultUri) );

  client->subscribeEvent (callbackToken, playerEndPoint, g_KmsMediaPlayerEndPointType_constants.EVENT_EOS, "", 0);
  GST_DEBUG ("callbackToken: %s", callbackToken.c_str () );
  client->unsubscribeEvent (playerEndPoint, callbackToken);

  try {
    client->subscribeEvent (callbackToken, playerEndPoint, "BAD_EVENT_TYPE", "", 0);
    BOOST_FAIL ("Subscribe for an event not supported by PlayerEndPoint must throw a KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_EVENT_NOT_SUPPORTED, e.errorCode);
  }

  client->release (mediaPipeline);
}

static void
check_recorder_end_point (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef recorderEndPoint = KmsMediaObjectRef();
  std::map<std::string, KmsMediaParam> params;
  KmsMediaInvocationReturn ret;
  std::string originalUri = "file:///tmp/player_end_point_test.webm";
  std::string resultUri;

  client->createMediaPipeline (mediaPipeline);
  params = createKmsMediaUriEndPointConstructorParams (originalUri);
  client->createMediaElementWithParams (recorderEndPoint, mediaPipeline, g_KmsMediaRecorderEndPointType_constants.TYPE_NAME, params);

  client->invoke (ret, recorderEndPoint, g_KmsMediaUriEndPointType_constants.GET_URI, emptyParams);

  BOOST_REQUIRE_NO_THROW (resultUri = unmarshalStringInvocationReturn (ret) );;
  BOOST_CHECK_EQUAL (0, originalUri.compare (resultUri) );

  client->release (mediaPipeline);
}

static void
check_http_end_point (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef httpEp = KmsMediaObjectRef();
  KmsMediaInvocationReturn ret;

  client->createMediaPipeline (mediaPipeline);
  client->createMediaElement (httpEp, mediaPipeline, g_KmsMediaHttpEndPointType_constants.TYPE_NAME);
  client->invoke (ret, httpEp, g_KmsMediaHttpEndPointType_constants.GET_URL, emptyParams);

  GST_INFO ("HttpEndPoint URL: %s", unmarshalStringInvocationReturn (ret).c_str () );

  client->release (mediaPipeline);
}

static void
check_zbar_filter (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef zbarFilter = KmsMediaObjectRef();

  client->createMediaPipeline (mediaPipeline);
  client->createMediaElement (zbarFilter, mediaPipeline, g_KmsMediaZBarFilterType_constants.TYPE_NAME);
  client->release (mediaPipeline);
}

static void
check_jackvader_filter (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef jackVaderFilter = KmsMediaObjectRef();

  client->createMediaPipeline (mediaPipeline);
  client->createMediaElement (jackVaderFilter, mediaPipeline, g_KmsMediaJackVaderFilterType_constants.TYPE_NAME);
  client->release (mediaPipeline);
}

static void
client_side (boost::shared_ptr<kurento::KmsMediaServerServiceClient> client)
{
  check_version (client);
  check_use_released_media_pipeline (client);
  check_auto_released_media_pipeline (client);
  check_keep_alive_media_pipeline (client);
  check_exclude_from_gc (client);
  check_parent (client);
  check_get_parent_of_media_pipeline (client);
  check_getMediaPipeline (client);
  check_same_token (client);
  check_get_media_element_from_pad (client);

#if 0 /* Temporally disabled */
  check_type (client);
#endif

  check_player_end_point (client);
  check_recorder_end_point (client);
  check_http_end_point (client);
  check_zbar_filter (client);
  check_jackvader_filter (client);
}

BOOST_AUTO_TEST_CASE ( server_test )
{
  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0, GST_DEFAULT_NAME);
  client_side (client);
}

BOOST_AUTO_TEST_SUITE_END()
