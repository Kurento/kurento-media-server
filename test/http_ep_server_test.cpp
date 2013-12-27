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
#define DISCONNECTION_TIMEOUT 2 /* seconds */

#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_OPTIONS "OPTIONS"

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
init_test_case ()
{
  loop = NULL;
  urls = NULL;
  urls_registered = 0;
  signal_count = 0;
  counted = 0;

  setenv ("GST_PLUGIN_PATH", "./plugins", TRUE);
  gst_init (NULL, NULL);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  loop = g_main_loop_new (NULL, FALSE);
  session = soup_session_async_new();

  /* Start Http End Point Server */
  httpepserver = kms_http_ep_server_new (KMS_HTTP_EP_SERVER_PORT, DEFAULT_PORT,
                                         KMS_HTTP_EP_SERVER_INTERFACE, DEFAULT_HOST, NULL);
}

static void
tear_down_test_case ()
{
  /* check for missed unrefs before exiting */
  // TODO: Enable check again when leak in 32 bits is found
  // BOOST_CHECK ( G_OBJECT (httpepserver)->ref_count == 1 );

  g_object_unref (G_OBJECT (httpepserver) );
  g_object_unref (G_OBJECT (session) );
  g_slist_free_full (urls, g_free);
  g_main_loop_unref (loop);
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
    url = kms_http_ep_server_register_end_point (httpepserver, httpep,
          DISCONNECTION_TIMEOUT);

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
    g_main_loop_quit (loop);
    BOOST_FAIL ( "Http server could not start" );
    return;
  }

  register_http_end_points (MAX_REGISTERED_HTTP_END_POINTS);

  session_cb = http_req_callback;

  g_idle_add ( (GSourceFunc) checking_registered_urls, &expected_200);
}

BOOST_AUTO_TEST_CASE ( register_http_end_point_test )
{
  init_test_case ();

  GST_INFO ("Running test register_http_end_point_test");

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

  tear_down_test_case ();
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
    g_main_loop_quit (loop);
    BOOST_FAIL ( "Http server could not start" );
    return;
  }

  register_http_end_points (MAX_REGISTERED_HTTP_END_POINTS);

  session_cb = t2_http_req_callback;

  g_idle_add ( (GSourceFunc) checking_registered_urls, &expected_200);
}

BOOST_AUTO_TEST_CASE ( locked_get_request_http_end_point_test )
{
  init_test_case ();

  GST_INFO ("Running locked_get_request_http_end_point_test");

  g_signal_connect (httpepserver, "url-removed", G_CALLBACK (t2_url_removed_cb),
                    NULL);
  g_signal_connect (httpepserver, "action-requested",
                    G_CALLBACK (t2_action_requested_cb), NULL);

  kms_http_ep_server_start (httpepserver, t2_http_server_start_cb);

  g_main_loop_run (loop);

  GST_DEBUG ("Test finished");

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);
  tear_down_test_case ();
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
    g_main_loop_quit (loop);
    BOOST_FAIL ( "Http server could not start" );
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
  init_test_case ();

  GST_INFO ("Running shutdown_http_end_point_test");

  g_signal_connect (httpepserver, "url-removed", G_CALLBACK (t3_url_removed_cb),
                    NULL);
  g_signal_connect (httpepserver, "action-requested",
                    G_CALLBACK (t3_action_requested_cb), NULL);

  kms_http_ep_server_start (httpepserver, t3_http_server_start_cb);

  g_main_loop_run (loop);

  GST_DEBUG ("Test finished");

  tear_down_test_case ();
}

/********************************************/
/* Functions and variables used for test 4  */
/********************************************/

static void
t4_http_req_callback (SoupSession *session, SoupMessage *msg, gpointer data)
{
  SoupKnownStatusCode *expected = (SoupKnownStatusCode *) data;
  guint status_code;
  gchar *method;
  SoupURI *uri;
  const gchar *cookie_str;

  g_object_get (G_OBJECT (msg), "method", &method, "status-code",
                &status_code, "uri", &uri, NULL);

  GST_DEBUG ("%s %s status code: %d, expected %d", method, soup_uri_get_path (uri),
             status_code, *expected);
  BOOST_CHECK (status_code == *expected);

  /* TODO: Check why soup_cookies_from_response does not work */
  cookie_str = soup_message_headers_get_list (msg->response_headers, "Set-Cookie");
  BOOST_CHECK (cookie_str != NULL);

  if (++counted == urls_registered)
    g_main_loop_quit (loop);

  soup_uri_free (uri);
  g_free (method);
}

static void
t4_http_server_start_cb (KmsHttpEPServer *self, GError *err)
{
  BOOST_CHECK ( err == NULL );

  if (err != NULL) {
    GST_ERROR ("%s, code %d", err->message, err->code);
    g_main_loop_quit (loop);
    BOOST_FAIL ( "Http server could not start" );
    return;
  }

  register_http_end_points (MAX_REGISTERED_HTTP_END_POINTS);

  session_cb = t4_http_req_callback;

  g_idle_add ( (GSourceFunc) checking_registered_urls, &expected_200);
}

static void
t4_action_requested_cb (KmsHttpEPServer *server, const gchar *uri,
                        KmsHttpEndPointAction action, gpointer data)
{
  GST_DEBUG ("Action %d requested on %s", action, uri);

  /* We unregister httpendpoints when they have already a pending request */
  /* so as to check we don't miss memory leaks */
  BOOST_CHECK (kms_http_ep_server_unregister_end_point (httpepserver, uri) );
}

BOOST_AUTO_TEST_CASE ( cookie_http_end_point_test )
{
  init_test_case ();

  GST_INFO ("Running cookie_http_end_point_test");

  g_signal_connect (httpepserver, "action-requested",
                    G_CALLBACK (t4_action_requested_cb), NULL);

  kms_http_ep_server_start (httpepserver, t4_http_server_start_cb);

  g_main_loop_run (loop);

  GST_DEBUG ("Test finished");

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);
  tear_down_test_case ();
}

/********************************************/
/* Functions and variables used for test 5  */
/********************************************/

static GstElement *pipeline, *httpep;
static const gchar *t5_uri;
static SoupCookie *cookie;

static void
bus_msg_cb (GstBus *bus, GstMessage *msg, gpointer pipeline)
{
  switch (msg->type) {
  case GST_MESSAGE_ERROR: {
    GST_ERROR ("%s bus error: %" GST_PTR_FORMAT, GST_ELEMENT_NAME (pipeline),
               msg);
    BOOST_FAIL ("Error received on the bus");
    break;
  }

  case GST_MESSAGE_WARNING: {
    GST_WARNING ("%s bus: %" GST_PTR_FORMAT, GST_ELEMENT_NAME (pipeline),
                 msg);
    break;
  }

  default:
    break;
  }
}

static void
t5_request_no_cookie_cb (SoupSession *session, SoupMessage *msg,
                         gpointer user_data)
{
  GST_DEBUG ("status code: %d", msg->status_code);

  /* Request should not be attended without the proper cookie */
  if (msg->status_code != SOUP_STATUS_BAD_REQUEST)
    BOOST_FAIL ("Get request without cookie failed");

  g_main_loop_quit (loop);
}

static void
t5_make_request_without_cookie()
{
  SoupMessage *msg;
  gchar *url;

  url = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, t5_uri);

  GST_INFO ("Send " HTTP_GET " %s", url);

  msg = soup_message_new (HTTP_GET, url);
  soup_session_queue_message (session, msg, t5_request_no_cookie_cb, NULL);

  g_free (url);
}

static gboolean
t5_cancel_cb (gpointer data)
{
  SoupMessage *msg = (SoupMessage *) data;
  GST_DEBUG ("Cancel Message.");
  soup_session_cancel_message (session, msg, SOUP_STATUS_CANCELLED);

  return FALSE;
}

static void
t5_request_with_cookie_cb (SoupSession *session, SoupMessage *msg,
                           gpointer user_data)
{
  guint status_code;
  gchar *method;
  SoupURI *uri;

  GST_DEBUG ("Request with cookie");

  g_object_get (G_OBJECT (msg), "method", &method, "status-code",
                &status_code, "uri", &uri, NULL);

  GST_WARNING ("%s %s status code: %d, expected %d", method,
               soup_uri_get_path (uri), status_code, SOUP_STATUS_CANCELLED);

  BOOST_CHECK (status_code == SOUP_STATUS_CANCELLED);

  t5_make_request_without_cookie();

  g_free (method);
  soup_uri_free (uri);
}

static void
t5_send_get_request_2 ()
{
  SoupMessage *msg;
  gchar *url, *header;

  url = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, t5_uri);

  GST_INFO ("Send " HTTP_GET " %s", url);
  msg = soup_message_new (HTTP_GET, url);

  header = soup_cookie_to_cookie_header (cookie);
  soup_message_headers_append (msg->request_headers, "Cookie", header);
  g_free (header);

  soup_session_queue_message (session, msg, t5_request_with_cookie_cb, NULL);

  g_timeout_add_full (G_PRIORITY_DEFAULT, 1000, t5_cancel_cb,
                      g_object_ref (G_OBJECT (msg) ), g_object_unref);
  g_free (url);
}

static void
t5_http_req_callback (SoupSession *session, SoupMessage *msg, gpointer data)
{
  guint status_code;
  gchar *method;
  SoupURI *uri;
  const gchar *header;

  g_object_get (G_OBJECT (msg), "method", &method, "status-code",
                &status_code, "uri", &uri, NULL);

  GST_WARNING ("%s %s status code: %d, expected %d", method, soup_uri_get_path (uri),
               status_code, SOUP_STATUS_CANCELLED);
  BOOST_CHECK (status_code == SOUP_STATUS_CANCELLED);

  /* TODO: Check why soup_cookies_from_response does not work */
  header = soup_message_headers_get_list (msg->response_headers, "Set-Cookie");

  BOOST_CHECK (header != NULL);

  cookie = soup_cookie_parse (header, NULL);
  BOOST_CHECK (cookie != NULL);

  t5_send_get_request_2();

  soup_uri_free (uri);
  g_free (method);
}

static void
t5_send_get_request_1 ()
{
  SoupMessage *msg;
  gchar *url;

  url = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, t5_uri);

  GST_INFO ("Send " HTTP_GET " %s", url);
  msg = soup_message_new (HTTP_GET, url);
  soup_session_queue_message (session, msg, t5_http_req_callback, NULL);

  g_timeout_add_full (G_PRIORITY_DEFAULT, 1000, t5_cancel_cb,
                      g_object_ref (G_OBJECT (msg) ), g_object_unref);
  g_free (url);
}

static void
t5_http_server_start_cb (KmsHttpEPServer *self, GError *err)
{
  if (err != NULL) {
    GST_ERROR ("%s, code %d", err->message, err->code);
    g_main_loop_quit (loop);
    BOOST_FAIL ( "Http server could not start" );
    return;
  }

  GST_DEBUG ("Registering %s", GST_ELEMENT_NAME (httpep) );
  t5_uri = kms_http_ep_server_register_end_point (httpepserver, httpep,
           DISCONNECTION_TIMEOUT);
  BOOST_CHECK (t5_uri != NULL);

  if (t5_uri == NULL) {
    g_main_loop_quit (loop);
    return;
  }

  GST_DEBUG ("Registered url: %s", t5_uri);
  urls = g_slist_prepend (urls, (gpointer *) g_strdup (t5_uri) );

  t5_send_get_request_1 ();
}

static void
t5_url_removed_cb (KmsHttpEPServer *server, const gchar *url, gpointer data)
{
  GST_DEBUG ("URL %s removed", url);
}

static void
t5_action_requested_cb (KmsHttpEPServer *server, const gchar *uri,
                        KmsHttpEndPointAction action, gpointer data)
{
  GST_DEBUG ("Action %d requested on %s", action, uri);
  BOOST_CHECK ( action == KMS_HTTP_END_POINT_ACTION_GET );

  if (++counted == 1) {
    /* First time */
    GST_DEBUG ("Starting pipeline");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
  }
}

BOOST_AUTO_TEST_CASE ( expired_cookie_http_end_point_test )
{
  GstElement *videotestsrc, *timeoverlay, *encoder, *agnosticbin;
  guint bus_watch_id1;
  GstBus *srcbus;

  init_test_case ();

  GST_INFO ("Running expired_cookie_http_end_point_test");

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("src-pipeline");
  videotestsrc = gst_element_factory_make ("videotestsrc", NULL);
  encoder = gst_element_factory_make ("vp8enc", NULL);
  agnosticbin = gst_element_factory_make ("agnosticbin", NULL);
  timeoverlay = gst_element_factory_make ("timeoverlay", NULL);
  httpep = gst_element_factory_make ("httpendpoint", NULL);

  GST_DEBUG ("Adding watcher to the pipeline");
  srcbus = gst_pipeline_get_bus (GST_PIPELINE (pipeline) );

  bus_watch_id1 = gst_bus_add_watch (srcbus, gst_bus_async_signal_func, NULL);
  g_signal_connect (srcbus, "message", G_CALLBACK (bus_msg_cb), pipeline);
  g_object_unref (srcbus);

  GST_DEBUG ("Configuring source pipeline");
  gst_bin_add_many (GST_BIN (pipeline), videotestsrc, timeoverlay,
                    encoder, agnosticbin, httpep, NULL);
  gst_element_link (videotestsrc, timeoverlay);
  gst_element_link (timeoverlay, encoder);
  gst_element_link (encoder, agnosticbin);
  gst_element_link_pads (agnosticbin, NULL, httpep, "video_sink");

  GST_DEBUG ("Configuring elements");
  g_object_set (G_OBJECT (videotestsrc), "is-live", TRUE, "do-timestamp", TRUE,
                "pattern", 18, NULL);
  g_object_set (G_OBJECT (timeoverlay), "font-desc", "Sans 28", NULL);

  g_signal_connect (httpepserver, "url-removed", G_CALLBACK (t5_url_removed_cb),
                    NULL);
  g_signal_connect (httpepserver, "action-requested",
                    G_CALLBACK (t5_action_requested_cb), NULL);

  kms_http_ep_server_start (httpepserver, t5_http_server_start_cb);

  g_main_loop_run (loop);

  BOOST_CHECK_EQUAL (signal_count, urls_registered);

  GST_DEBUG ("Test finished");

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline) );
  g_source_remove (bus_watch_id1);

  tear_down_test_case ();
}


/**********************************/
/*           POST tests           */
/**********************************/

#define TEST_PARAM_URI "test-param-url"

static guint use_cases = 7;
static guint cases_counted = 0;

static const gchar *boundary = "----WebKitFormBoundaryaxGqVIPtg0lAjCrV";
static const gchar *body1 =
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"mytext\"\r\n"
  "\r\n"
  "This text must be ignored\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"testName\"\r\n"
  "\r\n"
  "This text is processsed because of the filename attribute in headers\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV--\r\n";

static const gchar *body2 =
  "\r\n------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"testName\"\r\n"
  "\r\n"
  "Test with a body starting with a blank line preceding the boundary\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV--\r\n";

static const gchar *body3 =
  "This is the preamble.  It is to be ignored, though it "
  "is a handy place for mail composers to include an "
  "explanatory note to non-MIME compliant readers.\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"testName\"\r\n"
  "\r\n"
  "This is the key data in the body with preamble\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"mytext\"\r\n"
  "\r\n"
  "This text must be ignored\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV--\r\n";

static const gchar *body4 =
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"mytext\"\r\n"
  "\r\n"
  "This text must be ignored\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"name1\"\r\n"
  "\r\n"
  "This is the first piece of data that uses filename attribute\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"name2\"\r\n"
  "\r\n"
  "This portion of data should be ignored\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV--\r\n";

static const gchar *body5 =
  "--This is the preamble starting with two hyphens.\r\n"
  "This preamble conatins new lines characters and \r\n"
  "it also contains the boundary ------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"mytext\"\r\n"
  "\r\n"
  "This text must be ignored\r\n"
  "\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"name1\"\r\n"
  "\r\n"
  "Valid data\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"name2\"\r\n"
  "\r\n"
  "This portion of data should be ignored\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV--\r\n";

static const gchar *body6 =
  "--This is the preamble starting with two hyphens.\r\n"
  "This preamble conatins new lines characters and \r\n"
  "it also contains the boundary ------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV\r\n"
  "Content-Disposition: form-data; name=\"testFile\"; filename=\"name1\"\r\n"
  "\r\n"
  "Valid data with only a portion of data in the multipart body\r\n"
  "------WebKitFormBoundaryaxGqVIPtg0lAjCrV--\r\n";

static void
check_test_finish ()
{
  if (++cases_counted != use_cases)
    return;

  GST_INFO ("Finishing test");
  g_main_loop_quit (loop);
}

static GstElement *
register_http_endpoint ()
{
  GstElement *httpep;
  const gchar *uri;

  httpep = gst_element_factory_make ("httpendpoint", NULL);
  BOOST_CHECK ( httpep != NULL );

  uri = kms_http_ep_server_register_end_point (httpepserver, httpep,
        DISCONNECTION_TIMEOUT);
  BOOST_CHECK (uri != NULL);

  g_object_set_data_full (G_OBJECT (httpep), TEST_PARAM_URI,
                          g_strndup (uri, strlen (uri) ), g_free);

  return httpep;
}

static void
unregister_http_endpoint (GstElement *httpep)
{
  gchar *uri;

  uri = (gchar *) g_object_get_data (G_OBJECT (httpep), TEST_PARAM_URI);
  BOOST_CHECK (uri != NULL);

  BOOST_CHECK (kms_http_ep_server_unregister_end_point (httpepserver, uri) );
}

static void
http_post_req_expected_failed (SoupSession *session, SoupMessage *msg, gpointer data)
{
  GstElement *httpep = GST_ELEMENT (data);

  BOOST_CHECK (msg->status_code == SOUP_STATUS_NOT_ACCEPTABLE);

  unregister_http_endpoint (httpep);

  check_test_finish ();
}

static void
http_post_req (SoupSession *session, SoupMessage *msg, gpointer data)
{
  GstElement *httpep = GST_ELEMENT (data);

  BOOST_CHECK (msg->status_code == SOUP_STATUS_OK);

  unregister_http_endpoint (httpep);

  check_test_finish ();
}

static void
send_malformed_post_request ()
{
  GstElement *httpep;
  SoupMessage *msg;
  gchar *url, *uri;

  httpep = register_http_endpoint ();
  uri = (gchar *) g_object_get_data (G_OBJECT (httpep), TEST_PARAM_URI);

  url = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, uri);

  GST_INFO ("Send " HTTP_POST " %s", url);

  msg = soup_message_new (HTTP_POST, url);
  soup_message_headers_set_content_type (msg->request_headers,
                                         "multipart/form-data",
                                         NULL);
  soup_session_queue_message (session, msg,
                              http_post_req_expected_failed, httpep);

  g_free (url);
}

static void
send_post_request (const gchar *body)
{
  GstElement *httpep;
  GHashTable *params;
  SoupMessage *msg;
  gchar *url, *uri;

  httpep = register_http_endpoint ();
  uri = (gchar *) g_object_get_data (G_OBJECT (httpep), TEST_PARAM_URI);

  url = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, uri);
  params = g_hash_table_new_full (g_str_hash,  g_str_equal, g_free, g_free);
  g_hash_table_insert (params,
                       g_strdup_printf ("%s", "boundary"),
                       g_strdup_printf ("%s", boundary) );

  GST_INFO ("Send " HTTP_POST " %s", url);

  msg = soup_message_new (HTTP_POST, url);
  soup_message_set_request (msg, "multipart/form-data", SOUP_MEMORY_STATIC,
                            body, strlen (body) );
  soup_message_headers_set_content_type (msg->request_headers,
                                         "multipart/form-data",
                                         params);
  soup_session_queue_message (session, msg, http_post_req, httpep);

  g_free (url);
  g_hash_table_unref (params);
}

static void
post_http_server_start_cb (KmsHttpEPServer *self, GError *err)
{
  if (err != NULL) {
    GST_ERROR ("%s, code %d", err->message, err->code);
    g_main_loop_quit (loop);
    BOOST_FAIL ( "Http server could not start" );
    return;
  }

  send_malformed_post_request ();
  send_post_request (body1);
  send_post_request (body2);
  send_post_request (body3);
  send_post_request (body4);
  send_post_request (body5);
  send_post_request (body6);
}

BOOST_AUTO_TEST_CASE ( post_http_end_point_test )
{
  init_test_case ();

  GST_INFO ("Running post_http_end_point_test");

  kms_http_ep_server_start (httpepserver, post_http_server_start_cb);

  g_main_loop_run (loop);

  GST_DEBUG ("Test finished");

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);
  tear_down_test_case ();
}

/**********************************/
/*         Options tests          */
/**********************************/

#define KMS_HTTP_END_POINT_METHOD_GET 0
#define KMS_HTTP_END_POINT_METHOD_POST 1

static GstElement *get_pipeline;
static GstElement *get_http;
const gchar *allow_expected = "GET, POST";

static void
options_register_http_endpoint (GstElement *httpep)
{
  const gchar *uri;

  uri = kms_http_ep_server_register_end_point (httpepserver, httpep,
        DISCONNECTION_TIMEOUT);
  BOOST_CHECK (uri != NULL);

  g_object_set_data_full (G_OBJECT (httpep), TEST_PARAM_URI,
                          g_strndup (uri, strlen (uri) ), g_free);
}

static void
create_get_pipeline ()
{
  GstElement *videotestsrc, *audiotestsrc, *timeoverlay, *encoder, *aencoder;
  guint method;

  get_pipeline = gst_pipeline_new ("src-pipeline");
  videotestsrc = gst_element_factory_make ("videotestsrc", NULL);
  encoder = gst_element_factory_make ("vp8enc", NULL);
  aencoder = gst_element_factory_make ("vorbisenc", NULL);
  timeoverlay = gst_element_factory_make ("timeoverlay", NULL);
  audiotestsrc = gst_element_factory_make ("audiotestsrc", NULL);
  get_http = gst_element_factory_make ("httpendpoint", NULL);

  GST_DEBUG ("Configuring GET pipeline");
  gst_bin_add_many (GST_BIN (get_pipeline), videotestsrc, timeoverlay,
                    audiotestsrc, encoder, aencoder, get_http, NULL);
  gst_element_link (videotestsrc, timeoverlay);
  gst_element_link (timeoverlay, encoder);
  gst_element_link (audiotestsrc, aencoder);
  gst_element_link_pads (encoder, NULL, get_http, "video_sink");
  gst_element_link_pads (aencoder, NULL, get_http, "audio_sink");

  gst_element_set_state (get_pipeline, GST_STATE_READY);

  g_object_get (G_OBJECT (get_http), "http-method", &method, NULL);
  GST_INFO ("Http end point configured as %d", method);
  BOOST_CHECK (method == KMS_HTTP_END_POINT_METHOD_GET);

  options_register_http_endpoint (get_http);
}

static void
http_options_req (SoupSession *session, SoupMessage *msg, gpointer data)
{
  const gchar *expected = (gchar *) data;
  const gchar *method;

  BOOST_CHECK (msg->status_code == SOUP_STATUS_OK);

  method = soup_message_headers_get_one (msg->response_headers, "Allow");
  BOOST_CHECK (method != NULL);

  BOOST_CHECK (g_strcmp0 (method, expected) == 0);

  g_main_loop_quit (loop);
}

static void
send_option_request (GstElement *httpep, const gchar *expected)
{
  SoupMessage *msg;
  gchar *url, *uri;

  uri = (gchar *) g_object_get_data (G_OBJECT (httpep), TEST_PARAM_URI);

  url = g_strdup_printf ("http://%s:%d%s", DEFAULT_HOST, DEFAULT_PORT, uri);

  GST_INFO ("Send " HTTP_OPTIONS " %s", url);

  msg = soup_message_new (HTTP_OPTIONS, url);

  soup_session_queue_message (session, msg, http_options_req,
                              (gpointer) expected);

  g_free (url);
}

static void
options_http_server_start_cb (KmsHttpEPServer *self, GError *err)
{
  if (err != NULL) {
    GST_ERROR ("%s, code %d", err->message, err->code);
    g_main_loop_quit (loop);
    BOOST_FAIL ( "Http server could not start" );
    return;
  }

  create_get_pipeline ();
  send_option_request (get_http, allow_expected);
}

BOOST_AUTO_TEST_CASE ( options_http_end_point_test )
{
  init_test_case ();

  GST_INFO ("Running options_http_end_point_test");

  kms_http_ep_server_start (httpepserver, options_http_server_start_cb);

  g_main_loop_run (loop);

  GST_DEBUG ("Test finished");

  gst_element_set_state (get_pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (get_pipeline) );

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);
  tear_down_test_case ();
}

BOOST_AUTO_TEST_SUITE_END()
