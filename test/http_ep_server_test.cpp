/*
 * http_ep_server_test.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
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
#define BOOST_TEST_MODULE http_ep_server
#include <boost/test/unit_test.hpp>

#include <gst/gst.h>
#include <libsoup/soup.h>
#include <KmsHttpEPServer.h>

#define GST_CAT_DEFAULT _http_ep_server_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "http_ep_server_test"

#define MAX_REGISTERED_HTTP_END_POINTS 5

#define HTTP_GET "GET"
#define DEFAULT_PORT 9091
#define DEFAULT_HOST "localhost"

static KmsHttpEPServer *httpepserver;
static GMainLoop *loop = NULL;
static GSList *urls = NULL;
static SoupSession *session;
static guint req_done = 0;

BOOST_AUTO_TEST_SUITE (http_ep_server_test)

static void
register_http_end_points()
{
  const gchar *url;
  gint i;

  for (i = 0; i < MAX_REGISTERED_HTTP_END_POINTS; i++) {
    /* TODO: Create a real HttpEndPoint element here */
    url = kms_http_ep_server_register_end_point (httpepserver, NULL, NULL);
    GST_DEBUG ("Registered url: %s", url);
    urls = g_slist_prepend (urls, (gpointer *) url);
  }
}

static void
http_req_callback (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
  guint status_code;
  gchar *method;
  SoupURI *uri;

  g_object_get (G_OBJECT (msg), "method", &method, "status-code",
      &status_code, "uri", &uri, NULL);

  GST_DEBUG ("%s %s status code: %d", method, soup_uri_get_path (uri), status_code);

  if (++req_done == MAX_REGISTERED_HTTP_END_POINTS)
    g_main_loop_quit (loop);

  soup_uri_free (uri);
  g_free (method);
}

static void
send_get_request (gchar *url, gpointer user_data)
{
  SoupMessage *msg;
  gchar *uri;

  uri = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, url);

  GST_DEBUG ("Send " HTTP_GET " %s", uri);
  msg = soup_message_new (HTTP_GET, uri);
  soup_session_queue_message (session, msg, (SoupSessionCallback) http_req_callback, NULL);

  g_free (uri);
}

static gboolean
checking_registered_urls (gpointer data)
{
  GST_DEBUG ("Sending GET request to all urls registered");

  g_slist_foreach (urls, (GFunc) send_get_request, NULL);

  return FALSE;
}

BOOST_AUTO_TEST_CASE ( register_http_end_pooint_test )
{
  gst_init (NULL, NULL);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  loop = g_main_loop_new (NULL, FALSE);
  session = soup_session_async_new();

  /* Start Http End Point Server */
  httpepserver = kms_http_ep_server_new (KMS_HTTP_EP_SERVER_PORT, DEFAULT_PORT,
      KMS_HTTP_EP_SERVER_INTERFACE, DEFAULT_HOST, NULL);
  kms_http_ep_server_start (httpepserver);

  register_http_end_points();

  g_idle_add ( (GSourceFunc) checking_registered_urls, NULL);

  g_main_loop_run (loop);

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);
  g_object_unref (G_OBJECT (httpepserver) );
  g_object_unref (G_OBJECT (session) );
  g_slist_free (urls);
}

BOOST_AUTO_TEST_SUITE_END()