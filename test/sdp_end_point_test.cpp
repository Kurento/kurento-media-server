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

#include "KmsMediaErrorCodes_constants.h"
#include "KmsMediaRtpEndPointType_constants.h"
#include "KmsMediaSdpEndPointType_constants.h"

#include "utils/marshalling.hpp"
#include "gst/gst.h"

#define GST_CAT_DEFAULT _sdp_end_point_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "sdp_end_point_test"

using namespace kurento;

BOOST_FIXTURE_TEST_SUITE ( sdp_end_point_test_suite,  F)

BOOST_AUTO_TEST_CASE ( rtp_end_point_test )
{
  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef sdpEpA = KmsMediaObjectRef();
  KmsMediaObjectRef sdpEpB = KmsMediaObjectRef();
  std::string sdp, localSdpA, remoteSdpA, localSdpB, remoteSdpB;
  KmsMediaCommand command;
  KmsMediaCommandResult result;

  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0, GST_DEFAULT_NAME);

  GST_INFO ( "--------- rtp_end_point_test start ---------");

  BOOST_REQUIRE_NO_THROW (client->createMediaPipeline (mediaPipeline) );

  BOOST_REQUIRE_NO_THROW (client->createMediaElement (sdpEpA, mediaPipeline, g_KmsMediaRtpEndPointType_constants.TYPE_NAME) );
  BOOST_REQUIRE_NO_THROW (client->createMediaElement (sdpEpB, mediaPipeline, g_KmsMediaRtpEndPointType_constants.TYPE_NAME) );

  command = * createVoidCommand (g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP);

  try {
    client->sendCommand (result, sdpEpA, command);
    BOOST_FAIL ("Get local SDP without negotiation must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_LOCAL_SDP_ERROR, e.errorCode);
  }

  command = * createVoidCommand (g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP);

  try {
    client->sendCommand (result, sdpEpA, command);
    BOOST_FAIL ("Get local SDP without negotiation must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_REMOTE_SDP_ERROR, e.errorCode);
  }

  command = * createVoidCommand (g_KmsMediaSdpEndPointType_constants.GENERATE_SDP_OFFER);
  BOOST_REQUIRE_NO_THROW (client->sendCommand (result, sdpEpA, command) );
  BOOST_REQUIRE_NO_THROW (sdp = unmarshalStringCommandResult (result) );

  command = * createStringCommand (g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER, sdp);
  BOOST_REQUIRE_NO_THROW (client->sendCommand (result, sdpEpB, command) );
  BOOST_REQUIRE_NO_THROW (sdp = unmarshalStringCommandResult (result) );

  command = * createStringCommand (g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_ANSWER, sdp);
  BOOST_REQUIRE_NO_THROW (client->sendCommand (result, sdpEpA, command) );
  BOOST_REQUIRE_NO_THROW (sdp = unmarshalStringCommandResult (result) );

  command = * createVoidCommand (g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP);
  BOOST_REQUIRE_NO_THROW (client->sendCommand (result, sdpEpA, command) );
  BOOST_REQUIRE_NO_THROW (localSdpA = unmarshalStringCommandResult (result) );

  command = * createVoidCommand (g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP);
  BOOST_REQUIRE_NO_THROW (client->sendCommand (result, sdpEpA, command) );
  BOOST_REQUIRE_NO_THROW (remoteSdpA = unmarshalStringCommandResult (result) );

  command = * createVoidCommand (g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP);
  BOOST_REQUIRE_NO_THROW (client->sendCommand (result, sdpEpB, command) );
  BOOST_REQUIRE_NO_THROW (localSdpB = unmarshalStringCommandResult (result) );

  command = * createVoidCommand (g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP);
  BOOST_REQUIRE_NO_THROW (client->sendCommand (result, sdpEpB, command) );
  BOOST_REQUIRE_NO_THROW (remoteSdpB = unmarshalStringCommandResult (result) );

  BOOST_CHECK_EQUAL (localSdpA, remoteSdpB);
  BOOST_CHECK_EQUAL (localSdpB, remoteSdpA);

  GST_INFO ( "--------- rtp_end_point_test end ---------");
}

BOOST_AUTO_TEST_SUITE_END()
