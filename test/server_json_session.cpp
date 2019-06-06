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

#define GST_CAT_DEFAULT _server_json_session
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_server_json_session"

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
  void check_create_pipeline_call ();
  void check_close_session ();
};

void
ClientHandler::check_create_pipeline_call()
{
  Json::Value request;
  Json::Value response;

  std::string pipeId;
  std::string playerId;
  std::string filterId;
  std::string recorderId1;
  std::string recorderId2;

  Json::Value params;
  Json::Value constructorParams;
  Json::Value operationParams;

  std::string sessionId1 = "1111";
  std::string sessionId2 = "2222";

  request["jsonrpc"] = "2.0";
  request["method"] = "create";

  /* Create a new piepeline using session 1 */

  params["type"] = "MediaPipeline";
  params["sessionId"] = sessionId1;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  pipeId = response["result"]["value"].asString();

  /* Create a player using session 1 */

  params["type"] = "PlayerEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = VIDEO_URI;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  playerId = response["result"]["value"].asString();
  constructorParams.clear();

  /* Create a filter using session 1 */

  params["type"] = "WebRtcEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  filterId = response["result"]["value"].asString();
  constructorParams.clear();

  /* Create a recorder using session 1 */

  params["type"] = "RecorderEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = RECORDER_URI_1;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  recorderId1 = response["result"]["value"].asString();
  constructorParams.clear();

  /* Create a recorder using session 2 */

  params["type"] = "RecorderEndpoint";
  params["sessionId"] = sessionId2;
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = RECORDER_URI_2;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  recorderId2 = response["result"]["value"].asString();
  constructorParams.clear();

  GST_DEBUG ("Pipeline: %s", pipeId.c_str() );
  GST_DEBUG ("Player: %s", playerId.c_str() );
  GST_DEBUG ("Filter: %s", filterId.c_str() );
  GST_DEBUG ("Recorder1: %s", recorderId1.c_str() );
  GST_DEBUG ("Recorder2: %s", recorderId2.c_str() );

  /* Link player and filter using session 1 */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId1;
  params["object"] = playerId;
  params["operation"] = "connect";

  operationParams["sink"] = filterId;

  params["operationParams"] = operationParams;
  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();

  /* Link filter and recorder1 using session 1 */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId1;
  params["object"] = filterId;
  params["operation"] = "connect";

  operationParams["sink"] = recorderId1;

  params["operationParams"] = operationParams;
  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();

  /* Link filter and recorder2 using session 2 */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId2;
  params["object"] = filterId;
  params["operation"] = "connect";

  operationParams["sink"] = recorderId2;

  params["operationParams"] = operationParams;
  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();

  /* Unref recorder1 using session 1 */

  request["method"] = "unref";
  params["sessionId"] = sessionId1;
  params["object"] = recorderId1;

  request["id"] = getId();
  request["params"] = params;
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Unref filter using session 1 */

  request["method"] = "unref";
  params["sessionId"] = sessionId1;
  params["object"] = filterId;

  request["params"] = params;
  request["id"] = getId();
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Unref player using session 1 */

  request["method"] = "unref";
  params["sessionId"] = sessionId1;
  params["object"] = playerId;

  request["id"] = getId();
  request["params"] = params;
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Unref recorder2 using session 2 */

  request["method"] = "unref";
  params["sessionId"] = sessionId2;
  params["object"] = recorderId2;

  request["id"] = getId();
  request["params"] = params;
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
}

void
ClientHandler::check_close_session ()
{
  Json::Value request;
  Json::Value response;

  std::string pipeId;
  std::string playerId;
  std::string filterId;
  std::string recorderId1;
  std::string recorderId2;

  Json::Value params;
  Json::Value constructorParams;
  Json::Value operationParams;

  std::string sessionId1 = "1111";
  std::string sessionId2 = "2222";
  std::string sessionId3 = "3333";

  request["jsonrpc"] = "2.0";
  request["method"] = "create";

  /* Create a new piepeline using session 1 */

  params["type"] = "MediaPipeline";
  params["sessionId"] = sessionId1;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  pipeId = response["result"]["value"].asString();

  /* Create a player using session 1 */

  params["type"] = "PlayerEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = VIDEO_URI;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  playerId = response["result"]["value"].asString();
  constructorParams.clear();

  /* Create a filter using session 1 */

  params["type"] = "WebRtcEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  filterId = response["result"]["value"].asString();
  constructorParams.clear();

  /* Create a recorder using session 1 */

  params["type"] = "RecorderEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = RECORDER_URI_1;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  recorderId1 = response["result"]["value"].asString();
  constructorParams.clear();

  /* Create a recorder using session 2 */

  params["type"] = "RecorderEndpoint";
  params["sessionId"] = sessionId2;
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = RECORDER_URI_2;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  recorderId2 = response["result"]["value"].asString();
  constructorParams.clear();

  GST_DEBUG ("Pipeline: %s", pipeId.c_str() );
  GST_DEBUG ("Player: %s", playerId.c_str() );
  GST_DEBUG ("Filter: %s", filterId.c_str() );
  GST_DEBUG ("Recorder1: %s", recorderId1.c_str() );
  GST_DEBUG ("Recorder2: %s", recorderId2.c_str() );

  /* Link player and filter using session 1 */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId1;
  params["object"] = playerId;
  params["operation"] = "connect";

  operationParams["sink"] = filterId;

  params["operationParams"] = operationParams;
  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();

  /* Link filter and recorder1 using session 1 */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId1;
  params["object"] = filterId;
  params["operation"] = "connect";

  operationParams["sink"] = recorderId1;

  params["operationParams"] = operationParams;
  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();

  /* Link filter and recorder2 using session 2 */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId2;
  params["object"] = filterId;
  params["operation"] = "connect";

  operationParams["sink"] = recorderId2;

  params["operationParams"] = operationParams;
  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();

  /* Test objects with session 3, there are 4 elements in pipeline,
   * pipeline is now referenced by session3 */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId3;
  params["object"] = pipeId;
  params["operation"] = "getChilds";

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isArray() );
  BOOST_CHECK (response["result"]["value"].size() == 4);

  /* CloseSession1 */

  request["method"] = "closeSession";
  params["sessionId"] = sessionId1;

  request["id"] = getId();
  request["params"] = params;
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Test objects with session 3, all objects only referenced by
   * session1 are deleted, there should be only 2 elements */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId3;
  params["object"] = pipeId;
  params["operation"] = "getChilds";

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isArray() );
  BOOST_CHECK (response["result"]["value"].size() == 2);

  /* CloseSession2 */

  request["method"] = "closeSession";
  params["sessionId"] = sessionId2;

  request["id"] = getId();
  request["params"] = params;
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Test objects with session 2, this refs again pipeline from session2,
   * but pipeline should be empty */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId2;
  params["object"] = pipeId;
  params["operation"] = "getChilds";

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isArray() );
  BOOST_CHECK (response["result"]["value"].size() == 0);

  /* CloseSession2 Releasing */

  request["method"] = "closeSession";
  params["sessionId"] = sessionId2;
  params["release"] = true;

  request["id"] = getId();
  request["params"] = params;
  response = sendRequest (request);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Test objects with session 3, pipeline should not exists */

  request["method"] = "invoke";
  params.clear();
  params["sessionId"] = sessionId3;
  params["object"] = pipeId;
  params["operation"] = "getChilds";

  request["id"] = getId();
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("error") );
  BOOST_CHECK (!response.isMember ("result") );
  BOOST_CHECK (response["error"].isMember ("code") );
  BOOST_CHECK (response["error"]["code"].isInt() );
  BOOST_CHECK (response["error"]["code"].asInt() == MEDIA_OBJECT_NOT_FOUND );
}

BOOST_FIXTURE_TEST_SUITE ( server_json_session_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_json_session )
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  start();
  check_create_pipeline_call();
  check_close_session ();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
