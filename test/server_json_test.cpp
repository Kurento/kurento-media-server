/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "BaseTest.hpp"
#include <boost/test/unit_test.hpp>
#include <KurentoException.hpp>

#include <gst/gst.h>

#include <json/json.h>

#define GST_CAT_DEFAULT _server_json_test_
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_server_json_test"

namespace kurento
{

class ClientHandler : public F
{
public:
  ClientHandler() : F()
  {
    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                             GST_DEFAULT_NAME);
  };

  ~ClientHandler() override = default;

protected:
  void check_error_call ();
  void check_create_pipeline_call ();
  void check_connect_call ();
  void check_bad_transaction_call ();
  void check_transaction_call ();

  void runTests ()
  {
    start ();
    check_connect_call();
    check_error_call();
    check_create_pipeline_call();
    check_bad_transaction_call();
    check_transaction_call();
  }
};

void
ClientHandler::check_error_call()
{
  Json::Value request;
  Json::Value response;
  Json::Reader reader;

  request["jsonrpc"] = "1";
  request["id"] = getId();
  request["method"] = "create";

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("error") );
  BOOST_CHECK (response["error"].isObject() );
  BOOST_CHECK (response["error"].isMember ("code") );
  BOOST_CHECK (response["error"]["code"].isInt() );
  BOOST_CHECK (response["error"]["code"].asInt() == -32600);
  BOOST_CHECK (response["error"].isMember ("message") );
}

void
ClientHandler::check_connect_call()
{
  Json::Value request;
  Json::Value response;
  Json::Value params;

  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "connect";
  request["params"] = params;

  response = sendRequest (request);

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

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("error") );
  BOOST_CHECK (response["error"].isObject() );
  BOOST_CHECK (response["error"].isMember ("code") );
  BOOST_CHECK (response["error"]["code"].isInt() );
  BOOST_CHECK (response["error"]["code"].asInt() == INVALID_SESSION);
  BOOST_CHECK (response["error"].isMember ("message") );
  BOOST_CHECK (response["error"].isMember ("data") );
  BOOST_CHECK (response["error"]["data"].isMember ("type") );
  BOOST_CHECK (response["error"]["data"]["type"] == "INVALID_SESSION");
}

void
ClientHandler::check_bad_transaction_call()
{
  Json::Value response, request;
  Json::Reader reader;
  std::string req_str;

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
            "\"uri\":\"http://files.openvidu.io/video/format/small.webm\"},"
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
            "\"type\":\"WebRtcEndpoint\"}"
            "}"
            "],"
            "\"sessionId\":\"c58960d9-4cac-4036-ad2e-1aef26946dae\"}}";

  BOOST_REQUIRE (reader.parse (req_str, request) );
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].type() != Json::ValueType::nullValue);
}

void
ClientHandler::check_transaction_call()
{
  Json::Value response, request;
  Json::Reader reader;
  std::string req_str;

  req_str =
      "{\"id\":" + std::to_string(getId()) +
      R"(,"jsonrpc":"2.0","method":"transaction","params":{"operations":[{"id":0,"jsonrpc":"2.0","method":"create","params":{"constructorParams":{},"type":"MediaPipeline"}},{"id":1,"jsonrpc":"2.0","method":"create","params":{"constructorParams":{"mediaPipeline":"newref:0","uri":"http://files.openvidu.io/video/format/small.webm"},"type":"PlayerEndpoint"}},{"id":2,"jsonrpc":"2.0","method":"create","params":{"constructorParams":{"mediaPipeline":"newref:0"},"type":"WebRtcEndpoint"}},{"id":3,"jsonrpc":"2.0","method":"invoke","params":{"object":"newref:1","operation":"connect","operationParams":{"sink":"newref:2"}}}],"sessionId":"b2b81900-2902-4417-a552-973911efec4c"}})";

  BOOST_REQUIRE (reader.parse (req_str, request) );
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].type() != Json::ValueType::nullValue);
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].type() == Json::ValueType::arrayValue);
  BOOST_CHECK (response["result"]["value"].size() == 4);
  BOOST_CHECK (response["result"]["value"][3]["value"].type() ==
               Json::ValueType::nullValue);
}

void
ClientHandler::check_create_pipeline_call()
{
  Json::Value request;
  Json::Value response;
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

  response = sendRequest (request);

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

  response = sendRequest (request);

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

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  request["id"] = getId();
  request["method"] = "ref";
  params.clear();
  params["object"] = objId;
  params["sessionId"] = "12345";
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  request["id"] = getId();
  request["method"] = "describe";
  params.clear();
  params["object"] = objId;
  params["sessionId"] = "12345";
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isMember ("type") );
  BOOST_CHECK (response["result"]["type"].asString () == "WebRtcEndpoint" );
  BOOST_CHECK (response["result"].isMember ("qualifiedType") );
  BOOST_CHECK (response["result"]["qualifiedType"].asString () ==
               "kurento.WebRtcEndpoint" );
  BOOST_CHECK (response["result"].isMember ("hierarchy") );
  Json::Value hierarchy = response["result"]["hierarchy"];
  std::vector <std::string> expected_hierarchy;

  expected_hierarchy.emplace_back("kurento.BaseRtpEndpoint");
  expected_hierarchy.emplace_back("kurento.SdpEndpoint");
  expected_hierarchy.emplace_back("kurento.SessionEndpoint");
  expected_hierarchy.emplace_back("kurento.Endpoint");
  expected_hierarchy.emplace_back("kurento.MediaElement");
  expected_hierarchy.emplace_back("kurento.MediaObject");

  BOOST_REQUIRE (hierarchy.isArray() );

  for (uint i = 0; i < hierarchy.size(); i++) {
    BOOST_REQUIRE (hierarchy[i].isString() );
    BOOST_CHECK (hierarchy[i].asString() == expected_hierarchy[i]);
  }

  std::string sessionId = "123456";
  request.removeMember ("id");
  request["id"] = getId();
  request["method"] = "release";
  params.clear();
  params["object"] = objId;
  params["sessionId"] = sessionId;

  params["operationParams"] = operationParams;
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (response["result"]["sessionId"].asString () == sessionId );
}

BOOST_FIXTURE_TEST_SUITE ( server_json_test, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_json_test )
{
  runTests();
}

//BOOST_AUTO_TEST_CASE ( server_json_test_ipv6 )
//{
//  setWsHost ("ip6-localhost");

//  runTests();
//}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
