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

#include "BaseTest.hpp"
#include <boost/test/unit_test.hpp>

#include <gst/gst.h>

#include <jsoncpp/json/json.h>

#define GST_CAT_DEFAULT _server_json_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_server_json_test"

namespace kurento
{

class ClientHandler : public F
{
public:
  ClientHandler() : F() {};

  ~ClientHandler () {}

protected:
  void check_error_call ();
  void check_create_pipeline_call ();
  void check_connect_call ();
  void check_bad_transaction_call ();
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
  request["id"] = getId();
  request["method"] = "create";

  req_str = writer.write (request);

  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (response.isMember ("error") );
  BOOST_CHECK (response["error"].isObject() );
  BOOST_CHECK (response["error"].isMember ("code") );
  BOOST_CHECK (response["error"]["code"].isInt() );
  BOOST_CHECK (response["error"]["code"] == -32600);
  BOOST_CHECK (response["error"].isMember ("message") );
}

void
ClientHandler::check_connect_call()
{
  Json::Value request;
  Json::Value response;
  Json::FastWriter writer;
  Json::Reader reader;
  std::string req_str;
  std::string response_str;
  Json::Value params;

  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "connect";
  request["params"] = params;

  req_str = writer.write (request);

  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (response["result"]["sessionId"].isString() );
  BOOST_CHECK (response["result"].isMember ("serverId") );
  BOOST_CHECK (response["result"]["serverId"].isString() );

  request["id"] = getId();
  params["sessionId"] = "fakeSession";
  request["params"] = params;

  req_str = writer.write (request);

  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (response.isMember ("error") );
  BOOST_CHECK (response["error"].isObject() );
  BOOST_CHECK (response["error"].isMember ("code") );
  BOOST_CHECK (response["error"]["code"].isInt() );
  BOOST_CHECK (response["error"]["code"] == 40007);
  BOOST_CHECK (response["error"].isMember ("message") );
  BOOST_CHECK (response["error"].isMember ("data") );
  BOOST_CHECK (response["error"]["data"].isMember ("type") );
  BOOST_CHECK (response["error"]["data"]["type"] == "INVALID_SESSION");
}

void
ClientHandler::check_bad_transaction_call()
{
  Json::Value response;
  Json::Reader reader;
  std::string req_str;
  std::string response_str;

  req_str = "{"
            "\"id\": 50000,"
            "\"jsonrpc\":\"2.0\","
            "\"method\":\"transaction\","
            "\"params\":{"
            "\"operations\":["
            "{"
            "\"id\":0,"
            "\"jsonrpc\":\"2.0\","
            "\"method\":\"create\","
            "\"params\":{"
            "\"constructorParams\":{"
            "\"mediaPipeline\":\"5b96c1ad-46ba-4366-8241-fbc1cd0e9bbd\","
            "\"uri\":\"http://files.kurento.org/video/small.webm\"},"
            "\"transaction\":{"
            "\"_events\":{},"
            "\"_maxListeners\":10,"
            "\"domain\":null,"
            "\"members\":[]},"
            "\"type\":\"PlayerEndpoint\"}"
            "},"
            "{"
            "\"id\":1,"
            "\"jsonrpc\":\"2.0\","
            "\"method\":\"create\","
            "\"params\":{\""
            "constructorParams\":{"
            "\"mediaPipeline\":\"5b96c1ad-46ba-4366-8241-fbc1cd0e9bbd\"},"
            "\"transaction\":{"
            "\"_events\":{},"
            "\"_maxListeners\":10,"
            "\"domain\":null,"
            "\"members\":[]},"
            "\"type\":\"HttpGetEndpoint\"}"
            "}"
            "],"
            "\"sessionId\":\"c58960d9-4cac-4036-ad2e-1aef26946dae\"}}";

  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].type() != Json::ValueType::nullValue);
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
  request["id"] = getId();
  request["method"] = "create";

  params["type"] = "MediaPipeline";
  params["sessionId"] = "123456";

  request["params"] = params;
  request["sessionId"] = "sessionId";

  req_str = writer.write (request);

  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  pipeId = response["result"]["value"].asString();

  params["type"] = "WebRtcEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  params["constructorParams"] = constructorParams;
  params["sessionId"] = "123456";

  request["id"] = getId();
  request["params"] = params;

  req_str = writer.write (request);
  response_str.clear();

  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  objId = response["result"]["value"].asString();

  request["method"] = "invoke";
  params.clear();
  params["object"] = objId;
  params["operation"] = "getName";
  params["sessionId"] = "123456";

  params["operationParams"] = operationParams;
  request["id"] = getId();
  request["params"] = params;

  req_str = writer.write (request);
  response_str.clear();

  response_str = sendMessage (req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  request["id"] = getId();
  request["method"] = "ref";
  params.clear();
  params["object"] = objId;
  params["sessionId"] = "12345";
  request["params"] = params;

  req_str = writer.write (request);
  response_str.clear();

  response_str = sendMessage (req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  request["id"] = getId();
  request["method"] = "describe";
  params.clear();
  params["object"] = objId;
  params["sessionId"] = "12345";
  request["params"] = params;

  req_str = writer.write (request);
  response_str.clear();

  response_str = sendMessage (req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isMember ("type") );
  BOOST_CHECK (response["result"]["type"].asString () == "WebRtcEndpoint" );

  std::string sessionId = "123456";
  request.removeMember ("id");
  request["id"] = getId();
  request["method"] = "release";
  params.clear();
  params["object"] = objId;
  params["sessionId"] = sessionId;

  params["operationParams"] = operationParams;
  request["params"] = params;

  req_str = writer.write (request);
  response_str.clear();

  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (response["result"]["sessionId"].asString () == sessionId );
}


BOOST_FIXTURE_TEST_SUITE ( server_unexpected_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_unexpected_test )
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_connect_call();
  check_error_call();
  check_create_pipeline_call();
  check_bad_transaction_call();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
