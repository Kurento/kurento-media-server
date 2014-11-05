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

#define GST_CAT_DEFAULT _server_json_session
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_server_json_session"

#define VIDEO_URI "https://ci.kurento.com/video/sintel.webm"
#define RECORDER_URI_1 "file:///tmp/recorder1.webm"
#define RECORDER_URI_2 "file:///tmp/recorder2.webm"

namespace kurento
{

class ClientHandler : public F
{
public:
  ClientHandler() : F() {};

  virtual ~ClientHandler () {};

protected:
  void check_create_pipeline_call ();
};

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

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  pipeId = response["result"]["value"].asString();
  response_str.clear();

  /* Create a player using session 1 */

  params["type"] = "PlayerEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = VIDEO_URI;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  playerId = response["result"]["value"].asString();
  constructorParams.clear();
  response_str.clear();

  /* Create a filter using session 1 */

  params["type"] = "HttpGetEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  filterId = response["result"]["value"].asString();
  constructorParams.clear();
  response_str.clear();

  /* Create a recorder using session 1 */

  params["type"] = "RecorderEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = RECORDER_URI_1;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  recorderId1 = response["result"]["value"].asString();
  constructorParams.clear();
  response_str.clear();

  /* Create a recorder using session 2 */

  params["type"] = "RecorderEndpoint";
  params["sessionId"] = sessionId2;
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = RECORDER_URI_2;
  params["constructorParams"] = constructorParams;

  request["id"] = getId();
  request["params"] = params;

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  recorderId2 = response["result"]["value"].asString();
  constructorParams.clear();
  response_str.clear();

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

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();
  response_str.clear();

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

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();
  response_str.clear();

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

  req_str = writer.write (request);
  response_str = sendMessage (req_str);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );

  params.clear();
  operationParams.clear();
  response_str.clear();

  /* Unref recorder1 using session 1 */

  request["method"] = "unref";
  params["sessionId"] = sessionId1;
  params["object"] = recorderId1;

  request["id"] = getId();
  request["params"] = params;
  req_str = writer.write (request);
  response_str = sendMessage (req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Unref filter using session 1 */

  request["method"] = "unref";
  params["sessionId"] = sessionId1;
  params["object"] = filterId;

  request["params"] = params;
  request["id"] = getId();
  req_str = writer.write (request);
  response_str = sendMessage (req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Unref player using session 1 */

  request["method"] = "unref";
  params["sessionId"] = sessionId1;
  params["object"] = playerId;

  request["id"] = getId();
  request["params"] = params;
  req_str = writer.write (request);
  response_str = sendMessage (req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );

  /* Unref recorder2 using session 2 */

  request["method"] = "unref";
  params["sessionId"] = sessionId2;
  params["object"] = recorderId2;

  request["id"] = getId();
  request["params"] = params;
  req_str = writer.write (request);
  response_str = sendMessage (req_str);
  BOOST_CHECK (reader.parse (response_str, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
}

BOOST_FIXTURE_TEST_SUITE ( server_json_session_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_json_session )
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_create_pipeline_call();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
