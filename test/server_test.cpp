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

#include "media_config_loader.hpp"
#include "mediaServer_constants.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace kurento;

#define GST_CAT_DEFAULT _server_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_test"

BOOST_AUTO_TEST_SUITE ( server_test_suite )

static void
check_version (kurento::MediaServerServiceClient client)
{
  int32_t v;
  int32_t gotVersion;
  mediaServerConstants *c;

  c = new mediaServerConstants();
  v = c->VERSION;
  delete c;

  gotVersion = client.getVersion();
  BOOST_CHECK_EQUAL (gotVersion, v);
}

static void
check_same_token (kurento::MediaServerServiceClient client)
{
  MediaObject mediaManager = MediaObject();
  MediaObject mo = MediaObject();

  client.createMediaManager (mediaManager, 0);

  client.createMixer (mo, mediaManager, MixerType::type::MAIN_MIXER);
  BOOST_CHECK_EQUAL (mediaManager.token, mo.token);

  client.release (mediaManager);
}

static void
client_side ()
{
  boost::shared_ptr<TSocket> socket (new TSocket (MEDIA_SERVER_ADDRESS, MEDIA_SERVER_SERVICE_PORT) );
  boost::shared_ptr<TTransport> transport (new TFramedTransport (socket) );
  boost::shared_ptr<TProtocol> protocol (new TBinaryProtocol (transport) );
  kurento::MediaServerServiceClient client (protocol);

  transport->open ();

  check_version (client);
  check_same_token (client);

  transport->close ();
}

BOOST_AUTO_TEST_CASE ( server_test )
{
  gst_init (NULL, NULL);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  START_SERVER_TEST();
  client_side();
  STOP_SERVER_TEST();
}

BOOST_AUTO_TEST_SUITE_END()
