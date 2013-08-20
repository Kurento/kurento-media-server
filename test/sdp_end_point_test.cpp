/*
 * server_test.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "server_test_base.hpp"
#include <boost/test/unit_test.hpp>

#include "mediaServer_constants.h"

#include <gst/gst.h>

using namespace kurento;

#define GST_CAT_DEFAULT _sdp_end_point_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "sdp_end_point_test"

BOOST_FIXTURE_TEST_SUITE ( sdp_end_point_test_suite,  F)

BOOST_AUTO_TEST_CASE ( rtp_end_point_test )
{
  MediaObjectId mediaPipeline = MediaObjectId();
  MediaObjectId sdpEpA = MediaObjectId();
  MediaObjectId sdpEpB = MediaObjectId();
  std::string out, localSdpA, remoteSdpA, localSdpB, remoteSdpB;

  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0, GST_DEFAULT_NAME);

  GST_INFO ( "--------- rtp_end_point_test start ---------");

  BOOST_REQUIRE_NO_THROW (client->addHandlerAddress (0, "localhost", 2323) );
  BOOST_REQUIRE_NO_THROW (client->createMediaPipeline (mediaPipeline, 0) );

  BOOST_REQUIRE_NO_THROW (client->createSdpEndPoint (sdpEpA, mediaPipeline, SdpEndPointType::type::RTP_END_POINT) );
  BOOST_CHECK_THROW (client->getLocalSessionDescription (out, sdpEpA), MediaServerException);
  BOOST_CHECK_THROW (client->getRemoteSessionDescription (out, sdpEpA), MediaServerException);

  BOOST_REQUIRE_NO_THROW (client->createSdpEndPoint (sdpEpB, mediaPipeline, SdpEndPointType::type::RTP_END_POINT) );
  BOOST_CHECK_THROW (client->getLocalSessionDescription (out, sdpEpB), MediaServerException);
  BOOST_CHECK_THROW (client->getRemoteSessionDescription (out, sdpEpB), MediaServerException);

  BOOST_REQUIRE_NO_THROW (client->generateOffer (out, sdpEpA) );
  BOOST_REQUIRE_NO_THROW (client->processOffer (out, sdpEpB, out) );
  BOOST_REQUIRE_NO_THROW ( client->processAnswer (out, sdpEpA, out) );

  BOOST_REQUIRE_NO_THROW (client->getLocalSessionDescription (localSdpA, sdpEpA) );
  BOOST_REQUIRE_NO_THROW (client->getRemoteSessionDescription (remoteSdpA, sdpEpA) );
  BOOST_REQUIRE_NO_THROW (client->getLocalSessionDescription (localSdpB, sdpEpB) );
  BOOST_REQUIRE_NO_THROW (client->getRemoteSessionDescription (remoteSdpB, sdpEpB) );

  BOOST_CHECK_EQUAL (localSdpA, remoteSdpB);
  BOOST_CHECK_EQUAL (localSdpB, remoteSdpA);

  GST_INFO ( "--------- rtp_end_point_test end ---------");
}

BOOST_AUTO_TEST_SUITE_END()
