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

#include <gst/gst.h>

#include <json/json.h>

#define GST_CAT_DEFAULT server_duplicate_requests
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_server_duplicate_requests"

#define VIDEO_URI "http://files.openvidu.io/video/format/sintel.webm"
#define RECORDER_URI_1 "file:///tmp/recorder1.webm"
#define RECORDER_URI_2 "file:///tmp/recorder2.webm"

namespace kurento
{

class ClientHandler : public F
{
public:
  ClientHandler() : F() {};

  ~ClientHandler() override = default;

protected:
  void check_create_duplicate_requests_call ();
};

void
ClientHandler::check_create_duplicate_requests_call()
{
  Json::Value request;
  Json::Value response, response2;
  std::string pipeId;

  Json::Value params;
  Json::Value constructorParams;
  Json::Value operationParams;

  std::string sessionId1 = "1111";

  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "create";

  /* Create a new piepeline using session 1 */

  params["type"] = "MediaPipeline";
  params["sessionId"] = sessionId1;

  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  pipeId = response["result"]["value"].asString();

  /* This requests should be already cached. Let's do it again */

  response2 = sendRequest (request);

  /* Responses 1 and 2 should be the same. No new pipelines were created */
  /* So objects ID and everything else should be equal */
  BOOST_CHECK (response == response2);

  /* Create a player using session 1 */
  request["id"] = getId();
  params["type"] = "PlayerEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = VIDEO_URI;
  params["constructorParams"] = constructorParams;

  request["params"] = params;

  response = sendRequest (request);

  /* This requests should be already cached. Let's do it again */
  response2 = sendRequest (request);

  /* Responses 1 and 2 should be the same. No new pipelines were created */
  /* So objects ID and everything else should be equal */
  BOOST_CHECK (response == response2);
}

BOOST_FIXTURE_TEST_SUITE ( server_duplicate_reqs_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_duplicate_reqs )
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  start();
  check_create_duplicate_requests_call();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
