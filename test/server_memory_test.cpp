/*
 * server_memory_test.cpp - Kurento Media Server
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
#include "memory.hpp"

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE ServerTest

#include <sys/socket.h>
#include <arpa/inet.h>

#include "MediaServerService.h"

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include <gst/gst.h>

#include "media_config.hpp"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace kurento;

#define GST_CAT_DEFAULT _server_memory_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_memory_test"

#define MEMORY_TOLERANCE 1024

BOOST_AUTO_TEST_SUITE ( server_memory_test_suite )

static void
check_release_media_manager (kurento::MediaServerServiceClient client, int serverPid)
{
  MediaObject mediaManager = MediaObject();
  MediaObject mo = MediaObject();
  int i, maxMemorySize, currentMemorySize;

  for (i = 0; i < 10000; i++) {
    client.createMediaManager (mediaManager, 0);
    client.createSdpEndPoint (mo, mediaManager, SdpEndPointType::type::RTP_END_POINT);
    client.createSdpEndPoint (mo, mediaManager, SdpEndPointType::type::WEBRTC_END_POINT);
    client.release (mediaManager);

    if (i == 0)
      maxMemorySize = get_data_memory (serverPid) + MEMORY_TOLERANCE;

    if (i % 100 == 0) {
      currentMemorySize = get_data_memory (serverPid);
      BOOST_CHECK (currentMemorySize <= maxMemorySize);

      if (currentMemorySize > maxMemorySize)
        break;
    }
  }
}

static void
client_side (int serverPid)
{
  boost::shared_ptr<TSocket> socket (new TSocket (MEDIA_SERVER_ADDRESS, MEDIA_SERVER_SERVICE_PORT) );
  boost::shared_ptr<TTransport> transport (new TFramedTransport (socket) );
  boost::shared_ptr<TProtocol> protocol (new TBinaryProtocol (transport) );
  kurento::MediaServerServiceClient client (protocol);

  transport->open ();

  check_release_media_manager (client, serverPid);

  transport->close ();
}

BOOST_AUTO_TEST_CASE ( server_memory_test )
{
  gst_init (NULL, NULL);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  START_SERVER_TEST();
  GST_DEBUG ("client side...");
  client_side (GET_SERVER_PID() );
  STOP_SERVER_TEST();
}

BOOST_AUTO_TEST_SUITE_END()
