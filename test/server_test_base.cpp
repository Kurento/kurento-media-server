/*
 * server_test_base.cpp - Kurento Media Server
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

#define BOOST_TEST_MODULE ServerTest
#include <boost/test/unit_test.hpp>

#include <unistd.h>
#include <sys/wait.h>
#include <glib.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <transport/TSocket.h>
#include <protocol/TBinaryProtocol.h>

#include <gst/gst.h>

#include "media_config.hpp"
#include "mediaServer_types.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

#define GST_CAT_DEFAULT _server_test_base_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_test_base"

#define MAX_RETRIES 5

G_LOCK_DEFINE (mutex);

static void
sig_handler (int sig)
{
  if (sig == SIGCONT)
    G_UNLOCK (mutex);
}

int
start_server_test ()
{
  pid_t childpid = -1;
  gchar *conf_file, *conf_file_param;

  conf_file = getenv ("MEDIA_SERVER_CONF_FILE");

  if (conf_file == NULL) {
    return -1;
  }

  conf_file_param = g_strconcat ("--conf-file=", conf_file, NULL);
  signal (SIGCONT, sig_handler);

  G_LOCK (mutex);
  childpid = fork();

  if (childpid >= 0) {
    if (childpid == 0) {
      execl ("./server/kurento", "kurento", conf_file_param,  "--gst-plugin-path=../gst-kurento-plugins/src", NULL);
    } else {
      G_LOCK (mutex);
      G_UNLOCK (mutex);
    }
  } else {
    BOOST_FAIL ("Error executing server");
  }

  return childpid;
}

void
stop_server_test (int pid)
{
  int status;

  kill (pid, SIGINT);
  wait (&status);
}


F::F()
{
  int i;

  gst_init (NULL, NULL);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0, GST_DEFAULT_NAME);

  GST_DEBUG ("setup fixture");
  pid = start_server_test(); // TODO: check pid < 0

  boost::shared_ptr<TSocket> socket (new TSocket (MEDIA_SERVER_ADDRESS, MEDIA_SERVER_SERVICE_PORT) );
  transport = boost::shared_ptr<TTransport> (new TFramedTransport (socket) );
  boost::shared_ptr<TProtocol> protocol (new TBinaryProtocol (transport) );
  client = boost::shared_ptr<kurento::MediaServerServiceClient> (new kurento::MediaServerServiceClient (protocol) );

  for (i = 0; i < MAX_RETRIES; i++) {
    try {
      transport->open ();
      initialized = true;
      break;
    } catch (std::exception e) {
      GST_WARNING ("Error connecting to the server (retry %d/%d): %s", i + 1, MAX_RETRIES, e.what () );
      sleep (1);
    }
  }
}

F::~F()
{
  GST_DEBUG ("teardown fixture");

  if (initialized) {
    transport->close ();
  }

  stop_server_test (pid);
}
