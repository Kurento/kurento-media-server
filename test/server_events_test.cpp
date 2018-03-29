/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
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
  ClientHandler() : F() {};

  ~ClientHandler() override = default;

protected:
  void check_not_duplicated_event ();
};

static std::string
get_id_from_event (Json::Value &event)
{
  BOOST_CHECK (event.isMember ("params") );
  BOOST_CHECK (event["params"].isObject() );
  BOOST_CHECK (event["params"].isMember ("value") );
  BOOST_CHECK (event["params"]["value"].isMember ("type") );
  BOOST_CHECK (event["params"]["value"]["type"].asString() == "ObjectCreated" );
  BOOST_CHECK (event["params"]["value"].isMember ("data") );
  BOOST_CHECK (event["params"]["value"]["data"].isMember ("object") );
  return event["params"]["value"]["data"]["object"].asString();
}

void
ClientHandler::check_not_duplicated_event()
{
  Json::Value request;
  Json::Value response;
  Json::Value params;
  std::string sessionId;
  std::string subscriptionId, subscriptionId2;
  std::string receivedId, responseId;

  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "subscribe";

  params["object"] = "manager_ServerManager";
  params["type"] = "ObjectCreated";

  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  sessionId = response["result"]["sessionId"].asString();
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"].isMember ("value") );
  subscriptionId = response["result"]["value"].asString();
  BOOST_CHECK (!subscriptionId.empty() );

  params["sessionId"] = sessionId;
  request["params"] = params;
  request["id"] = getId();

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["result"]["sessionId"].asString() );
  BOOST_CHECK (response["result"].isMember ("value") );
  subscriptionId2 = response["result"]["value"].asString();
  BOOST_CHECK (!subscriptionId2.empty() );

  BOOST_CHECK (subscriptionId != subscriptionId2);

  std::thread listener ([this, &receivedId] () {
    try {
      Json::Value event = this->waifForEvent (std::chrono::seconds (2) );
      receivedId = get_id_from_event (event);
    } catch (kurento::KurentoException e) {
      BOOST_FAIL ("Expected event not received");
    }

    try {
      this->waifForEvent (std::chrono::seconds (2) );
      BOOST_FAIL ("Unexpected event");
    } catch (kurento::KurentoException e) {
    }
  });

  request.clear();
  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "create";

  params.clear();
  params["type"] = "MediaPipeline";
  params["sessionId"] = sessionId;
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["result"]["sessionId"].asString() );
  BOOST_CHECK (response["result"].isMember ("value") );

  responseId = response["result"]["value"].asString();

  listener.join();

  BOOST_CHECK (receivedId == responseId);

  // Unsubscribe first listener and wait for event

  request.clear();
  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "unsubscribe";

  params.clear();
  params["object"] = "manager_ServerManager";
  params["subscription"] = subscriptionId;
  params["sessionId"] = sessionId;

  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["result"]["sessionId"].asString() );

  std::thread listener2 ([this, &receivedId] () {
    try {
      Json::Value event = this->waifForEvent (std::chrono::seconds (2) );
      receivedId = get_id_from_event (event);
    } catch (kurento::KurentoException e) {
      BOOST_FAIL ("Expected event not received");
    }

    try {
      this->waifForEvent (std::chrono::seconds (2) );
      BOOST_FAIL ("Unexpected event");
    } catch (kurento::KurentoException e) {
    }
  });

  request.clear();
  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "create";

  params.clear();
  params["type"] = "MediaPipeline";
  params["sessionId"] = sessionId;
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["result"]["sessionId"].asString() );
  BOOST_CHECK (response["result"].isMember ("value") );
  BOOST_CHECK (response["result"].isMember ("value") );

  responseId = response["result"]["value"].asString();

  listener2.join();

  BOOST_CHECK (receivedId == responseId);

  request.clear();
  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "unsubscribe";

  params.clear();
  params["object"] = "manager_ServerManager";
  params["subscription"] = subscriptionId2;
  params["sessionId"] = sessionId;

  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["result"]["sessionId"].asString() );

  std::thread listener3 ([this] () {
    try {
      Json::Value event = this->waifForEvent (std::chrono::seconds (2) );
      BOOST_ERROR ("Unexpected event: " + event.toStyledString() );
    } catch (kurento::KurentoException e) {
    }
  });

  request.clear();
  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "create";

  params.clear();
  params["type"] = "MediaPipeline";
  params["sessionId"] = sessionId;
  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["result"]["sessionId"].asString() );
  BOOST_CHECK (response["result"].isMember ("value") );

  std::cout << response["result"]["value"].toStyledString() << std::endl;

  listener3.join();

  // Unsubscribe second listener and no event should be received
}

BOOST_FIXTURE_TEST_SUITE ( server_unexpected_test_suite, ClientHandler)

BOOST_AUTO_TEST_CASE ( server_unexpected_test )
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  start ();
  check_not_duplicated_event();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
