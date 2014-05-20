/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#include <gst/gst.h>
#include "HttpService.hpp"
#include "httpendpointserver.hpp"

#define GST_CAT_DEFAULT kurento_http_service
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHttpService"

#define HTTP_SERVICE_GROUP "HttpEPServer"
#define HTTP_SERVICE_ADDRESS "serverAddress"
#define HTTP_SERVICE_PORT "serverPort"
#define HTTP_SERVICE_ANNOUNCED_ADDRESS "announcedAddress"

#define DEFAULT_PORT 9091

namespace kurento
{

static void
check_port (int port)
{
  if (port <= 0 || port > G_MAXUSHORT) {
    throw Glib::KeyFileError (Glib::KeyFileError::PARSE, "Invalid value");
  }
}

HttpService::HttpService (Glib::KeyFile &confFile,
                          bool fixedPort) : Service (confFile)
{
  try {
    address = confFile.get_string (HTTP_SERVICE_GROUP, HTTP_SERVICE_ADDRESS);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Http end point server will be listening to all interfaces");
  }

  if (fixedPort) {
    try {
      port = confFile.get_integer (HTTP_SERVICE_GROUP, HTTP_SERVICE_PORT);
      check_port (port);
    } catch (const Glib::KeyFileError &err) {
      GST_ERROR ("%s", err.what ().c_str () );
      GST_WARNING ("Setting default port %d to http end point server",
                   DEFAULT_PORT);
      port = DEFAULT_PORT;
    }
  } else {
    port = 0;
    GST_INFO ("HttpService will start on any available port");
  }

  try {
    announcedAddr = confFile.get_string (HTTP_SERVICE_GROUP,
                                         HTTP_SERVICE_ANNOUNCED_ADDRESS);
  } catch (const Glib::KeyFileError &err) {
    GST_ERROR ("%s", err.what ().c_str () );
    GST_WARNING ("Http end point server will choose any available "
                 "IP address to compose URLs");
  }

  server = kms_http_ep_server_new (
             KMS_HTTP_EP_SERVER_PORT, port,
             KMS_HTTP_EP_SERVER_INTERFACE,
             (address.empty() ) ? NULL : address.c_str (),
             KMS_HTTP_EP_SERVER_ANNOUNCED_IP,
             (announcedAddr.empty() ) ? NULL : announcedAddr.c_str (),
             NULL);

  /* FIXME: Do not use an extern variable. Develop a proper register and
   * unregister mechanism instead. */
  httpepserver = KMS_HTTP_EP_SERVER (g_object_ref (G_OBJECT (server) ) );
}

static void
http_server_handler_cb (KmsHttpEPServer *self, GError *err, gpointer data)
{
  auto handler =
    reinterpret_cast < std::function < void (GError *err) > * > (data);

  (*handler) (err);
}

void HttpService::start ()
{
  Glib::Cond cond;
  Glib::Mutex mutex;
  bool finish = FALSE;
  bool error;

  GST_DEBUG ("starting service");

  std::function <void (GError *err) > startHandler = [&] (GError * err) {
    error = (err != NULL);

    if (error) {
      GST_ERROR ("Service could not start. (%s)", err->message);
    } else {
      GST_INFO ("Service successfully started");
    }

    mutex.lock();
    finish = TRUE;
    cond.signal();
    mutex.unlock();
  };

  kms_http_ep_server_start (server, http_server_handler_cb, &startHandler, NULL);

  mutex.lock();

  while (!finish) {
    cond.wait (mutex);
  }

  mutex.unlock();

  if (error) {
    /* Http server could not start */
    throw;
  }
}

void HttpService::stop ()
{
  Glib::Cond cond;
  Glib::Mutex mutex;
  bool finish = FALSE;
  bool error;

  GST_DEBUG ("stopping service");

  std::function <void (GError *err) > stopHandler = [&] (GError * err) {
    error = (err != NULL);

    if (error) {
      GST_ERROR ("Error stopping server. (%s)", err->message);
    } else {
      GST_INFO ("Service stopped");
    }

    mutex.lock();
    finish = TRUE;
    cond.signal();
    mutex.unlock();
  };

  kms_http_ep_server_stop (server, http_server_handler_cb, &stopHandler, NULL);

  mutex.lock();

  while (!finish) {
    cond.wait (mutex);
  }

  mutex.unlock();

  if (error) {
    /* Http server could not stop */
    throw;
  }
}

HttpService::~HttpService()
{
  GST_DEBUG ("Destroying HttpService");
  g_object_unref (G_OBJECT (server) );

  /* FIXME: Do not use an extern variable. Develop a proper register and
   * unregister mechanism instead. */
  g_object_unref (G_OBJECT (httpepserver) );
}

HttpService::StaticConstructor HttpService::staticConstructor;

HttpService::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
