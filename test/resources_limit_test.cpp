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

  ~ClientHandler () {}

protected:
  void check_system_overload ();

  void runTests ()
  {
    setResourceLimit (0.015);
    start ();

    check_system_overload();
  }
};

void
ClientHandler::check_system_overload()
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
  params["sessionId"] = "1234567";

  request["params"] = params;
  request["sessionId"] = "sessionId";

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  pipeId = response["result"]["value"].asString();

  int times = 0;

  do {

    params["type"] = "WebRtcEndpoint";
    constructorParams ["mediaPipeline"] = pipeId;
    params["constructorParams"] = constructorParams;
    params["sessionId"] = "1234567";

    request["id"] = getId();
    request["params"] = params;

    response = sendRequest (request);

    times ++;
  } while (!response.isMember ("error") );

  BOOST_CHECK (times > 0);
  BOOST_CHECK (response.isMember ("error") );

  BOOST_CHECK (response["error"]["data"]["type"].asString() ==
               "NOT_ENOUGH_RESOURCES");
}

BOOST_FIXTURE_TEST_SUITE ( server_json_test, ClientHandler)

BOOST_AUTO_TEST_CASE ( resource_limit )
{
  runTests();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
