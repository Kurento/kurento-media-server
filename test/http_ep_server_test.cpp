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
#include <KmsHttpEPServer.h>

#define GST_CAT_DEFAULT _http_ep_server_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "http_ep_server_test"

#define MAX_REGISTERED_HTTP_END_POINTS 5

static KmsHttpEPServer *httpepserver;

BOOST_AUTO_TEST_SUITE (http_ep_server_test)

static void
register_http_end_points()
{
  const gchar *id;
  gint i;

  for (i = 0; i < MAX_REGISTERED_HTTP_END_POINTS; i++) {
    /* TODO: Create a real HttpEndPoint element here */
    id = kms_http_ep_server_register_end_point (httpepserver, NULL, NULL);
    GST_DEBUG ("Registered url: %s", id);
  }
}

BOOST_AUTO_TEST_CASE ( register_http_end_pooint_test )
{
  gst_init (NULL, NULL);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0, GST_DEFAULT_NAME);

  /* Start Http End Point Server */
  httpepserver = kms_http_ep_server_new (NULL);
  kms_http_ep_server_start (httpepserver);

  register_http_end_points();

  /* Stop Http End Point Server and destroy it */
  kms_http_ep_server_stop (httpepserver);
  g_object_unref (G_OBJECT (httpepserver) );
}

BOOST_AUTO_TEST_SUITE_END()