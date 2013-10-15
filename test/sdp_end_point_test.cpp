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

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "gst/gst.h"

#define GST_CAT_DEFAULT _sdp_end_point_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "sdp_end_point_test"

using namespace kurento;

static std::map<std::string, KmsMediaParam> emptyParams = std::map<std::string, KmsMediaParam> ();

BOOST_FIXTURE_TEST_SUITE ( sdp_end_point_test_suite,  F)

BOOST_AUTO_TEST_CASE ( rtp_end_point_test )
{

  KmsMediaObjectRef mediaPipeline = KmsMediaObjectRef();
  KmsMediaObjectRef sdpEpA = KmsMediaObjectRef();
  KmsMediaObjectRef sdpEpB = KmsMediaObjectRef();
  std::string sdp, localSdpA, remoteSdpA, localSdpB, remoteSdpB;
  KmsMediaInvocationReturn ret;

  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0, GST_DEFAULT_NAME);

  GST_INFO ( "--------- rtp_end_point_test start ---------");

  BOOST_REQUIRE_NO_THROW (client->createMediaPipeline (mediaPipeline) );

  BOOST_REQUIRE_NO_THROW (client->createMediaElement (sdpEpA, mediaPipeline, g_KmsMediaRtpEndPointType_constants.TYPE_NAME) );
  BOOST_REQUIRE_NO_THROW (client->createMediaElement (sdpEpB, mediaPipeline, g_KmsMediaRtpEndPointType_constants.TYPE_NAME) );

  try {
    client->invoke (ret, sdpEpA, g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP, emptyParams);
    BOOST_FAIL ("Get local SDP without negotiation must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_LOCAL_SDP_ERROR, e.errorCode);
  }

  try {
    client->invoke (ret, sdpEpA, g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP, emptyParams);
    BOOST_FAIL ("Get local SDP without negotiation must throw KmsMediaServerException");
  } catch (const KmsMediaServerException &e) {
    BOOST_CHECK_EQUAL (g_KmsMediaErrorCodes_constants.SDP_END_POINT_NO_REMOTE_SDP_ERROR, e.errorCode);
  }

  BOOST_REQUIRE_NO_THROW (client->invoke (ret, sdpEpA, g_KmsMediaSdpEndPointType_constants.GENERATE_SDP_OFFER, emptyParams) );
  BOOST_REQUIRE_NO_THROW (sdp = unmarshalStringInvocationReturn (ret) );

  std::map<std::string, KmsMediaParam> paramsProcessOffer;
  setStringParam (paramsProcessOffer, g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER_PARAM_OFFER_STR, sdp);
  BOOST_REQUIRE_NO_THROW (client->invoke (ret, sdpEpB, g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_OFFER, paramsProcessOffer) );
  BOOST_REQUIRE_NO_THROW (sdp = unmarshalStringInvocationReturn (ret) );

  std::map<std::string, KmsMediaParam> paramsProcessAnswer;
  setStringParam (paramsProcessAnswer, g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_ANSWER_PARAM_ANSWER_STR, sdp);
  BOOST_REQUIRE_NO_THROW (client->invoke (ret, sdpEpA, g_KmsMediaSdpEndPointType_constants.PROCESS_SDP_ANSWER, paramsProcessAnswer) );
  BOOST_REQUIRE_NO_THROW (sdp = unmarshalStringInvocationReturn (ret) );

  BOOST_REQUIRE_NO_THROW (client->invoke (ret, sdpEpA, g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP, emptyParams) );
  BOOST_REQUIRE_NO_THROW (localSdpA = unmarshalStringInvocationReturn (ret) );

  BOOST_REQUIRE_NO_THROW (client->invoke (ret, sdpEpA, g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP, emptyParams) );
  BOOST_REQUIRE_NO_THROW (remoteSdpA = unmarshalStringInvocationReturn (ret) );

  BOOST_REQUIRE_NO_THROW (client->invoke (ret, sdpEpB, g_KmsMediaSdpEndPointType_constants.GET_LOCAL_SDP, emptyParams) );
  BOOST_REQUIRE_NO_THROW (localSdpB = unmarshalStringInvocationReturn (ret) );

  BOOST_REQUIRE_NO_THROW (client->invoke (ret, sdpEpB, g_KmsMediaSdpEndPointType_constants.GET_REMOTE_SDP, emptyParams) );
  BOOST_REQUIRE_NO_THROW (remoteSdpB = unmarshalStringInvocationReturn (ret) );

  BOOST_CHECK_EQUAL (localSdpA, remoteSdpB);
  BOOST_CHECK_EQUAL (localSdpB, remoteSdpA);

  GST_INFO ( "--------- rtp_end_point_test end ---------");
}

BOOST_AUTO_TEST_SUITE_END()
