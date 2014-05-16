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
#include "KmsMediaServer_types.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

#define GST_CAT_DEFAULT _server_test_base_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_test_base"

#define MEDIA_SERVER_ADDRESS "localhost"
#define MEDIA_SERVER_SERVICE_PORT 9090
#define MAX_RETRIES 5

static GMutex mutex;
static GCond cond;

static bool started = false;

static void
sig_handler (int sig)
{
  if (sig == SIGCONT) {
    g_mutex_lock (&mutex);
    started = true;
    g_cond_signal (&cond);
    g_mutex_unlock (&mutex);
  }
}

int
start_server_test ()
{
  pid_t childpid = -1;
  gchar *conf_file, *conf_file_param, *binary_dir;

  conf_file = getenv ("MEDIA_SERVER_CONF_FILE");
  binary_dir = getenv ("SERVER_DIR");

  if (conf_file == NULL) {
    return -1;
  }

  conf_file_param = g_strconcat ("--conf-file=", conf_file, NULL);
  signal (SIGCONT, sig_handler);

  childpid = fork();

  if (childpid >= 0) {
    if (childpid == 0) {
      execl (binary_dir, "kurento", conf_file_param,
             "--gst-plugin-path=.", NULL);
    } else {
      g_mutex_lock (&mutex);

      while (!started) {
        g_cond_wait (&cond, &mutex);
      }

      g_mutex_unlock (&mutex);
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

  if (pid != waitpid (pid, &status, 0) ) {
    BOOST_FAIL ("Error waiting for child process");
  }
}

F::F()
{
  int i;

  gst_init (NULL, NULL);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  GST_DEBUG ("setup fixture");
  pid = start_server_test(); // TODO: check pid < 0

  boost::shared_ptr<TSocket> socket (new TSocket (MEDIA_SERVER_ADDRESS,
                                     MEDIA_SERVER_SERVICE_PORT) );
  transport = boost::shared_ptr<TTransport> (new TFramedTransport (socket) );
  boost::shared_ptr<TProtocol> protocol (new TBinaryProtocol (transport) );
  client = boost::shared_ptr<kurento::KmsMediaServerServiceClient>
           (new kurento::KmsMediaServerServiceClient (protocol) );

  for (i = 0; i < MAX_RETRIES; i++) {
    try {
      transport->open ();
      initialized = true;
      break;
    } catch (const std::exception &e) {
      GST_WARNING ("Error connecting to the server (retry %d/%d): %s", i + 1,
                   MAX_RETRIES, e.what () );
      usleep (100000);
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
