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
  std::string interface;
  std::string announcedAddr;
  uint port;

  try {
    interface = confFile.get_string (HTTP_SERVICE_GROUP,
                                     HTTP_SERVICE_ADDRESS);
  } catch (const Glib::KeyFileError &err) {
    GST_WARNING ("Http end point server will be listening to all interfaces");
  }

  if (fixedPort) {
    try {
      port = confFile.get_integer (HTTP_SERVICE_GROUP, HTTP_SERVICE_PORT);
      check_port (port);
    } catch (const Glib::KeyFileError &err) {
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
    GST_WARNING ("Http end point server will choose any available "
                 "IP address to compose URLs");
  }

  if (!HttpEndPointServer::configure (port, interface, announcedAddr) ) {
    throw HttpServiceException ("Can not configure HTTP server");
  }
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

    finish = TRUE;
  };

  HttpEndPointServer::getHttpEndPointServer()->start (http_server_handler_cb,
      &startHandler, NULL);

  while (!finish) {
    /* Yeld the execution to next main loop iteration and block until done */
    g_main_context_iteration (NULL, TRUE);
  }

  if (error) {
    throw new HttpServiceException ("Http service could not start");
  }
}

void HttpService::stop ()
{
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

    finish = TRUE;
  };

  HttpEndPointServer::getHttpEndPointServer()->stop (http_server_handler_cb,
      &stopHandler, NULL);

  while (!finish) {
    /* Main loop is not running so we set might_block to FALSE */
    g_main_context_iteration (NULL, FALSE);
  }

  if (error) {
    throw new HttpServiceException ("Http service could not stop");
  }
}

HttpService::~HttpService()
{
}

HttpService::StaticConstructor HttpService::staticConstructor;

HttpService::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */