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

#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE TestExample

#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "MediaServerService.h"

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include <gst/gst.h>

#include "media_config_loader.hpp"
#include "types/MediaFactory.hpp"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace kurento;

#define GST_CAT_DEFAULT _server_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_test"

BOOST_AUTO_TEST_SUITE ( test_suite )

void
check_same_token (kurento::MediaServerServiceClient client)
{
  MediaObject mediaFactory = MediaObject();
  MediaObject mo = MediaObject();

  client.createMediaFactory (mediaFactory);

  client.createMediaPlayer (mo, mediaFactory);
  BOOST_CHECK_EQUAL (mediaFactory.token, mo.token);

  client.createMediaRecorder (mo, mediaFactory);
  BOOST_CHECK_EQUAL (mediaFactory.token, mo.token);

  client.createStream (mo, mediaFactory);
  BOOST_CHECK_EQUAL (mediaFactory.token, mo.token);

  client.createMixer (mo, mediaFactory, DefaultMixerType);
  BOOST_CHECK_EQUAL (mediaFactory.token, mo.token);

  client.createMixer (mo, mediaFactory, DummyMixerType);
  BOOST_CHECK_EQUAL (mediaFactory.token, mo.token);

  client.release (mediaFactory);
}

void
client_side ()
{
  boost::shared_ptr<TSocket> socket (new TSocket (MEDIA_SERVER_ADDRESS, MEDIA_SERVER_SERVICE_PORT) );
  boost::shared_ptr<TTransport> transport (new TFramedTransport (socket) );
  boost::shared_ptr<TProtocol> protocol (new TBinaryProtocol (transport) );
  kurento::MediaServerServiceClient client (protocol);

  transport->open ();

  check_same_token (client);

  transport->close ();
}

BOOST_AUTO_TEST_CASE ( server_test )
{
  pid_t childpid;
  int status;

  gst_init (NULL, NULL);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  childpid = fork();

  if (childpid >= 0) {
    if (childpid == 0) {
      execl ("./server/kurento", "kurento", NULL);
    } else {
      // TODO: Look for a better system to detect that server has started
      sleep (1);
      client_side();
      kill (childpid, SIGINT);
      wait (&status);
    }
  } else {
    BOOST_FAIL ("Error while forking");
  }
}

BOOST_AUTO_TEST_SUITE_END()
