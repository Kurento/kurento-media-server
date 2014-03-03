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

#define GST_CAT_DEFAULT _server_json_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "server_json_test"

class ClientHandler : public F
{
public:
  ClientHandler() : F() {};

  ~ClientHandler () {}

protected:
  void check_error_call ();
  void check_create_pipeline_call ();
};

void
ClientHandler::check_error_call()
{
  Json::Value request;
  Json::Value response;
  Json::FastWriter writer;
  Json::Reader reader;
  std::string req_str;
  std::string response_str;

  request["jsonrpc"] = "1";
  request["id"] = 0;
  request["method"] = "create";

  req_str = writer.write (request);

  client->invokeJsonRpc (response_str, req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (response.isMember ("error") );
  BOOST_CHECK (response["error"].isObject() );
  BOOST_CHECK (response["error"].isMember ("code") );
  BOOST_CHECK (response["error"]["code"].isInt() );
  BOOST_CHECK (response["error"]["code"] == -32600);
  BOOST_CHECK (response["error"].isMember ("message") );
}

void
ClientHandler::check_create_pipeline_call()
{
  Json::Value request;
  Json::Value response;
  Json::FastWriter writer;
  Json::Reader reader;
  std::string req_str;
  std::string response_str;
  std::string pipeId;
  std::string objId;

  Json::Value params;
  Json::Value constructorParams;
  Json::Value operationParams;

  request["jsonrpc"] = "2.0";
  request["id"] = 0;
  request["method"] = "create";

  params["type"] = "MediaPipeline";

  request["params"] = params;

  req_str = writer.write (request);

  client->invokeJsonRpc (response_str, req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isString() );

  pipeId = response["result"].asString();

  params["type"] = "WebRtcEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  params["constructorParams"] = constructorParams;

  request["params"] = params;

  req_str = writer.write (request);
  response_str.clear();

  client->invokeJsonRpc (response_str, req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isString() );

  objId = response["result"].asString();

  request["method"] = "invoke";
  params.clear();
  params["object"] = objId;
  params["operation"] = "getMediaSrcs";

  params["operationParams"] = operationParams;
  request["params"] = params;

  req_str = writer.write (request);
  response_str.clear();

  client->invokeJsonRpc (response_str, req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  // TODO: Perform more tests
}


BOOST_FIXTURE_TEST_SUITE ( server_unexpected_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_unexpected_test )
{
  BOOST_REQUIRE_MESSAGE (initialized, "Cannot connect to the server");
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_error_call();
  check_create_pipeline_call();
}

BOOST_AUTO_TEST_SUITE_END()
