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
#include <boost/test/unit_test.hpp>

#include <gst/gst.h>

#include <jsoncpp/json/json.h>

#define GST_CAT_DEFAULT _server_json_session
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_json_session"

class ClientHandler : public F
{
public:
  ClientHandler() : F() {};

  ~ClientHandler () {}

protected:
  void check_create_pipeline_call ();
};

void
ClientHandler::check_create_pipeline_call()
{
  /*TODO: */
}

BOOST_FIXTURE_TEST_SUITE ( server_json_session_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_json_session )
{
  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_create_pipeline_call();
}

BOOST_AUTO_TEST_SUITE_END()