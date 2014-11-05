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

#define GST_CAT_DEFAULT server_duplicate_requests
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "test_server_duplicate_requests"

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
  void check_create_duplicate_requests_call ();
};

void
ClientHandler::check_create_duplicate_requests_call()
{
  Json::Value request;
  Json::Value response;
  Json::FastWriter writer;
  Json::Reader reader;
  std::string req_str;
  std::string response_str1;
  std::string response_str2;

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

  req_str = writer.write (request);
  response_str1 = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str1, response) == true);

  BOOST_CHECK (!response.isMember ("error") );
  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"]["value"].isString() );

  pipeId = response["result"]["value"].asString();

  /* This requests should be already cached. Let's do it again */

  req_str = writer.write (request);
  response_str2 = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str2, response) == true);

  /* Responses 1 and 2 should be the same. No new pipelines were created */
  /* So objects ID and everything else should be equal */
  BOOST_CHECK (response_str1 == response_str2);

  response_str1.clear();
  response_str2.clear();

  /* Create a player using session 1 */
  request["id"] = getId();
  params["type"] = "PlayerEndpoint";
  constructorParams ["mediaPipeline"] = pipeId;
  constructorParams ["uri"] = VIDEO_URI;
  params["constructorParams"] = constructorParams;

  request["params"] = params;

  req_str = writer.write (request);
  response_str1 = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str1, response) == true);

  /* This requests should be already cached. Let's do it again */
  req_str = writer.write (request);
  response_str2 = sendMessage (req_str);

  BOOST_CHECK (reader.parse (response_str2, response) == true);

  /* Responses 1 and 2 should be the same. No new pipelines were created */
  /* So objects ID and everything else should be equal */
  BOOST_CHECK (response_str1 == response_str2);
}

BOOST_FIXTURE_TEST_SUITE ( server_duplicate_reqs_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_duplicate_reqs )
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  check_create_duplicate_requests_call();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
