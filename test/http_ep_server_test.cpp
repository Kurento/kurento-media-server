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
#include <kmshttpendpointaction.h>

#define GST_CAT_DEFAULT _http_endpoint_server_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "http_endpoint_server_test"

#define MAX_REGISTERED_HTTP_END_POINTS 10

#define HTTP_GET "GET"
#define DEFAULT_PORT 9091
#define DEFAULT_HOST "localhost"

static KmsHttpEPServer *httpepserver;
static SoupSession *session;

static GMainLoop *loop;
static GSList *urls;
static guint urls_registered;
static guint signal_count;
static guint counted;

static SoupKnownStatusCode expected_200 = SOUP_STATUS_OK;
static SoupKnownStatusCode expected_404 = SOUP_STATUS_NOT_FOUND;

SoupSessionCallback session_cb;


BOOST_AUTO_TEST_SUITE (http_ep_server_test)

static void
init_global_variables()
{
  loop = NULL;
  urls = NULL;
  urls_registered = 0;
  signal_count = 0;
  counted = 0;
}

static void
register_http_end_points (gint n)
{
  const gchar *url;
  gint i;

  for (i = 0; i < n; i++) {
    GstElement *httpep = gst_element_factory_make ("httpendpoint", NULL);
    BOOST_CHECK ( httpep != NULL );

    GST_DEBUG ("Registering %s", GST_ELEMENT_NAME (httpep) );
    url = kms_http_ep_server_register_end_point (httpepserver, httpep);

    BOOST_CHECK (url != NULL);

    if (url == NULL)
      continue;

    /* Leave the last reference to http end point server */
    g_object_unref (G_OBJECT (httpep) );

    GST_DEBUG ("Registered url: %s", url);
    urls = g_slist_prepend (urls, (gpointer *) g_strdup (url) );
  }

  urls_registered = g_slist_length (urls);
}

static void
http_req_callback (SoupSession *session, SoupMessage *msg, gpointer data)
{
  SoupKnownStatusCode *expected = (SoupKnownStatusCode *) data;
  guint status_code;
  gchar *method;
  SoupURI *uri;

  g_object_get (G_OBJECT (msg), "method", &method, "status-code",
      &status_code, "uri", &uri, NULL);

  GST_DEBUG ("%s %s status code: %d, expected %d", method, soup_uri_get_path (uri),
      status_code, *expected);

  BOOST_CHECK_EQUAL (*expected, status_code);

  if (*expected == expected_404 && ++counted == urls_registered)
    g_main_loop_quit (loop);

  soup_uri_free (uri);
  g_free (method);
}

static void
send_get_request (const gchar *uri, gpointer data)
{
  SoupMessage *msg;
  gchar *url;

  url = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, uri);

  GST_INFO ("Send " HTTP_GET " %s", url);
  msg = soup_message_new (HTTP_GET, url);
  soup_session_queue_message (session, msg, session_cb, data);

  g_free (url);
}

static gboolean
checking_registered_urls (gpointer data)
{
  GST_DEBUG ("Sending GET request to all urls registered");

  g_slist_foreach (urls, (GFunc) send_get_request, data);

  return FALSE;
}

static void
url_removed_cb (KmsHttpEPServer *server, const gchar *url, gpointer data)
{
  GST_DEBUG ("URL %s removed", url);

  if (++signal_count == urls_registered) {
    /* Testing removed URLs */
    g_idle_add ( (GSourceFunc) checking_registered_urls, &expected_404);
  }
}

static void
action_requested_cb (KmsHttpEPServer *server, const gchar *uri,
    KmsHttpEndPointAction action, gpointer data)
{
  GST_DEBUG ("Action %d requested on %s", action, uri);
  BOOST_CHECK ( action == KMS_HTTP_END_POINT_ACTION_GET );

  BOOST_CHECK (kms_http_ep_server_unregister_end_point (httpepserver, uri) );
}

static void
http_server_start_cb (KmsHttpEPServer *self, GError *err)
{
  if (err != NULL) {
    GST_ERROR ("%s, code %d", err->message, err->code);
    return;
  }

  register_http_end_points (MAX_REGISTERED_HTTP_END_POINTS);

  session_cb = http_req_callback;

  g_idle_add ( (GSourceFunc) checking_registered_urls, &expected_200);
}

BOOST_AUTO_TEST_CASE ( register_http_end_point_test )
{
  gchar *env;

  env = g_strdup ("GST_PLUGIN_PATH=./plugins");
  putenv (env);

  gst_init (NULL, NULL);
  init_global_variables();

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  loop = g_main_loop_new (NULL, FALSE);
  session = soup_session_async_new();

  /* Start Http End Point Server */
  httpepserver = kms_http_ep_server_new (KMS_HTTP_EP_SERVER_PORT, DEFAULT_PORT,
      KMS_HTTP_EP_SERVER_INTERFACE, DEFAULT_HOST, NULL);

  g_signal_connect (httpepserver, "url-removed", G_CALLBACK (url_removed_cb),
      NULL);
  g_signal_connect (httpepserver, "action-requested", G_CALLBACK (action_requested_cb),
      NULL);

  kms_http_ep_server_start (httpepserver, http_server_start_cb);

  g_main_loop_run (loop);

  BOOST_CHECK_EQUAL (signal_count, urls_registered);

  GST_DEBUG ("Test finished");

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);

  /* check for missed unrefs before exiting */
  BOOST_CHECK ( G_OBJECT (httpepserver)->ref_count == 1 );

  g_object_unref (G_OBJECT (httpepserver) );
  g_object_unref (G_OBJECT (session) );
  g_slist_free_full (urls, g_free);
  g_main_loop_unref (loop);
  g_free (env);
}

/********************************************/
/* Functions and variables used for tests 2 */
/********************************************/

static void
t2_http_req_callback (SoupSession *session, SoupMessage *msg, gpointer data)
{
  SoupKnownStatusCode *expected = (SoupKnownStatusCode *) data;
  guint status_code;
  gchar *method;
  SoupURI *uri;

  g_object_get (G_OBJECT (msg), "method", &method, "status-code",
      &status_code, "uri", &uri, NULL);

  GST_DEBUG ("%s %s status code: %d, expected %d", method, soup_uri_get_path (uri),
      status_code, *expected);

  BOOST_CHECK_EQUAL (*expected, status_code);

  if (++counted == urls_registered)
    g_main_loop_quit (loop);

  soup_uri_free (uri);
  g_free (method);
}

static void
t2_action_requested_cb (KmsHttpEPServer *server, const gchar *uri,
    KmsHttpEndPointAction action, gpointer data)
{
  GST_DEBUG ("Action %d requested on %s", action, uri);

  /* We unregister httpendpoints when they have already a pending request */
  /* so as to check we don't miss memory leaks */
  BOOST_CHECK (kms_http_ep_server_unregister_end_point (httpepserver, uri) );
}

static void
t2_url_removed_cb (KmsHttpEPServer *server, const gchar *url, gpointer data)
{
  GST_DEBUG ("URL %s removed", url);
}

static void
t2_http_server_start_cb (KmsHttpEPServer *self, GError *err)
{
  if (err != NULL) {
    GST_ERROR ("%s, code %d", err->message, err->code);
    return;
  }

  register_http_end_points (MAX_REGISTERED_HTTP_END_POINTS);

  session_cb = t2_http_req_callback;

  g_idle_add ( (GSourceFunc) checking_registered_urls, &expected_200);
}

BOOST_AUTO_TEST_CASE ( locked_get_request_http_end_point_test )
{
  gchar *env;

  env = g_strdup ("GST_PLUGIN_PATH=./plugins");
  putenv (env);

  gst_init (NULL, NULL);
  init_global_variables();

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  loop = g_main_loop_new (NULL, FALSE);
  session = soup_session_async_new();

  /* Start Http End Point Server */
  httpepserver = kms_http_ep_server_new (KMS_HTTP_EP_SERVER_PORT, DEFAULT_PORT,
      KMS_HTTP_EP_SERVER_INTERFACE, DEFAULT_HOST, NULL);

  g_signal_connect (httpepserver, "url-removed", G_CALLBACK (t2_url_removed_cb),
      NULL);
  g_signal_connect (httpepserver, "action-requested",
      G_CALLBACK (t2_action_requested_cb), NULL);

  kms_http_ep_server_start (httpepserver, t2_http_server_start_cb);

  g_main_loop_run (loop);

  GST_DEBUG ("Test finished");

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);

  /* check for missed unrefs before exiting */
  BOOST_CHECK ( G_OBJECT (httpepserver)->ref_count == 1 );

  g_object_unref (G_OBJECT (httpepserver) );
  g_object_unref (G_OBJECT (session) );
  g_slist_free_full (urls, g_free);
  g_main_loop_unref (loop);
  g_free (env);
}

/********************************************/
/* Functions and variables used for tests 3 */
/********************************************/

static guint QUEUED = 2;

static void
t3_http_req_callback (SoupSession *session, SoupMessage *msg, gpointer data)
{
  SoupKnownStatusCode *expected = (SoupKnownStatusCode *) data;
  guint status_code;
  gchar *method;
  SoupURI *uri;

  g_object_get (G_OBJECT (msg), "method", &method, "status-code",
      &status_code, "uri", &uri, NULL);

  GST_DEBUG ("%s %s status code: %d, expected %d", method, soup_uri_get_path (uri),
      status_code, *expected);

  BOOST_CHECK_EQUAL (*expected, status_code);

  if (++counted == urls_registered)
    g_main_loop_quit (loop);

  soup_uri_free (uri);
  g_free (method);
}

static void
t3_http_server_start_cb (KmsHttpEPServer *self, GError *err)
{
  if (err != NULL) {
    GST_ERROR ("%s, code %d", err->message, err->code);
    return;
  }

  register_http_end_points (QUEUED);

  session_cb = t3_http_req_callback;

  g_idle_add ( (GSourceFunc) checking_registered_urls, &expected_200);
}

static void
t3_action_requested_cb (KmsHttpEPServer *server, const gchar *uri,
    KmsHttpEndPointAction action, gpointer data)
{
  GST_DEBUG ("Action %d requested on %s", action, uri);

  if (++signal_count == QUEUED) {
    /* Stop Http End Point Server and destroy it */
    kms_http_ep_server_stop (httpepserver);
  }
}

static void
t3_url_removed_cb (KmsHttpEPServer *server, const gchar *url, gpointer data)
{
  GST_DEBUG ("URL %s removed", url);
}

BOOST_AUTO_TEST_CASE ( shutdown_http_end_point_test )
{
  gchar *env;

  env = g_strdup ("GST_PLUGIN_PATH=./plugins");
  putenv (env);

  gst_init (NULL, NULL);
  init_global_variables();

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  loop = g_main_loop_new (NULL, FALSE);
  session = soup_session_async_new();

  /* Start Http End Point Server */
  httpepserver = kms_http_ep_server_new (KMS_HTTP_EP_SERVER_PORT, DEFAULT_PORT,
      KMS_HTTP_EP_SERVER_INTERFACE, DEFAULT_HOST, NULL);

  g_signal_connect (httpepserver, "url-removed", G_CALLBACK (t3_url_removed_cb),
      NULL);
  g_signal_connect (httpepserver, "action-requested",
      G_CALLBACK (t3_action_requested_cb), NULL);

  kms_http_ep_server_start (httpepserver, t3_http_server_start_cb);

  g_main_loop_run (loop);

  GST_DEBUG ("Test finished");

  /* check for missed unrefs before exiting */
  BOOST_CHECK ( G_OBJECT (httpepserver)->ref_count == 1 );

  g_object_unref (G_OBJECT (httpepserver) );
  g_object_unref (G_OBJECT (session) );
  g_slist_free_full (urls, g_free);
  g_main_loop_unref (loop);
  g_free (env);
}
BOOST_AUTO_TEST_SUITE_END()