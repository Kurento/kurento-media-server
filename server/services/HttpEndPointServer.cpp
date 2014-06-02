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

#include "HttpEndPointServer.hpp"

#define GST_CAT_DEFAULT HttpEndPointServer_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "HttpEndPointServer"

using namespace Glib::Threads;

namespace kurento
{

std::shared_ptr<HttpEndPointServer> HttpEndPointServer::instance = 0;
RecMutex HttpEndPointServer::mutex;

uint HttpEndPointServer::port;
std::string HttpEndPointServer::interface;
std::string HttpEndPointServer::announcedAddr;

std::shared_ptr<HttpEndPointServer>
HttpEndPointServer::getHttpEndPointServer()
{
  RecMutex::Lock lock (mutex);

  if (!instance) {
    instance = std::shared_ptr<HttpEndPointServer> (new HttpEndPointServer () );
  }

  return instance;
}

bool
HttpEndPointServer::configure (uint port, std::string iface, std::string addr)
{
  RecMutex::Lock lock (mutex);

  if (instance) {
    return false;
  }

  HttpEndPointServer::port = port;
  HttpEndPointServer::interface = iface;
  HttpEndPointServer::announcedAddr = addr;

  return true;
}

HttpEndPointServer::HttpEndPointServer ()
{
  server = kms_http_ep_server_new (
             KMS_HTTP_EP_SERVER_PORT, HttpEndPointServer::port,
             KMS_HTTP_EP_SERVER_INTERFACE,
             (HttpEndPointServer::interface.empty() ) ? NULL :
             HttpEndPointServer::interface.c_str (),
             KMS_HTTP_EP_SERVER_ANNOUNCED_IP,
             (HttpEndPointServer::announcedAddr.empty() ) ? NULL :
             HttpEndPointServer::announcedAddr.c_str (),
             NULL);
}

HttpEndPointServer::~HttpEndPointServer()
{
  g_object_unref (G_OBJECT (server) );
}

void
HttpEndPointServer::start (KmsHttpEPServerNotifyCallback start_cb,
                           gpointer user_data,
                           GDestroyNotify notify)
{
  kms_http_ep_server_start (server, start_cb, user_data, notify);
}

void
HttpEndPointServer::stop (KmsHttpEPServerNotifyCallback stop_cb,
                          gpointer user_data,
                          GDestroyNotify notify)
{
  kms_http_ep_server_stop (server, stop_cb, user_data, notify);
}

void
HttpEndPointServer::registerEndPoint (GstElement *endpoint, guint timeout,
                                      KmsHttpEPRegisterCallback cb, gpointer user_data, GDestroyNotify notify)
{
  kms_http_ep_server_register_end_point (server, endpoint, timeout, cb, user_data,
                                         notify);
}

void
HttpEndPointServer::unregisterEndPoint (std::string uri,
                                        KmsHttpEPServerNotifyCallback cb, gpointer user_data, GDestroyNotify notify)
{
  kms_http_ep_server_unregister_end_point (server, uri.c_str(), cb, user_data,
      notify);
}

gulong
HttpEndPointServer::connectSignal (std::string name, GCallback c_handler,
                                   gpointer user_data)
{
  return g_signal_connect (server, name.c_str(), c_handler, user_data);
}

void
HttpEndPointServer::disconnectSignal (gulong id)
{
  g_signal_handler_disconnect (server, id);
}

uint
HttpEndPointServer::getPort ()
{
  guint port;

  g_object_get (G_OBJECT (server), KMS_HTTP_EP_SERVER_PORT, &port, NULL);

  return port;
}

std::string
HttpEndPointServer::getInterface()
{
  std::string iface;
  gchar *iface_c;

  g_object_get (G_OBJECT (server), KMS_HTTP_EP_SERVER_INTERFACE, &iface_c, NULL);
  iface = iface_c;
  g_free (iface_c);

  return iface;
}

std::string
HttpEndPointServer::getAnnouncedAddress()
{
  std::string addr;
  gchar *addr_c;

  g_object_get (G_OBJECT (server), KMS_HTTP_EP_SERVER_ANNOUNCED_IP, &addr_c,
                NULL);
  addr = addr_c;
  g_free (addr_c);

  return addr;
}

HttpEndPointServer::StaticConstructor HttpEndPointServer::staticConstructor;

HttpEndPointServer::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */