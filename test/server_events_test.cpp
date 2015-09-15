/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
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

  ~ClientHandler () {}

protected:
  void check_not_duplicated_event ();
};

void
ClientHandler::check_not_duplicated_event()
{
  Json::Value request;
  Json::Value response;
  Json::Value params;
  std::string sessionId;
  std::string subscriptionId, subscriptionId2;

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
  sessionId = response["retult"]["sessionId"].asString();
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
  BOOST_CHECK (sessionId == response["retult"]["sessionId"].asString() );
  BOOST_CHECK (response["result"].isMember ("value") );
  subscriptionId2 = response["result"]["value"].asString();
  BOOST_CHECK (!subscriptionId2.empty() );

  BOOST_CHECK (subscriptionId != subscriptionId2);

  std::thread listener ([this] () {
    try {
      this->waifForEvent (std::chrono::seconds (2) );
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
  BOOST_CHECK (sessionId == response["retult"]["sessionId"].asString() );
  BOOST_CHECK (response["result"].isMember ("value") );

  listener.join();

  // Unsubscribe first listener and wait for event

  request.clear();
  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "unsubscribe";

  params["object"] = "manager_ServerManager";
  params["subscription"] = subscriptionId;
  params["sessionId"] = sessionId;

  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["retult"]["sessionId"].asString() );

  std::thread listener2 ([this] () {
    try {
      this->waifForEvent (std::chrono::seconds (2) );
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
  BOOST_CHECK (sessionId == response["retult"]["sessionId"].asString() );
  BOOST_CHECK (response["result"].isMember ("value") );

  listener2.join();

  request.clear();
  request["jsonrpc"] = "2.0";
  request["id"] = getId();
  request["method"] = "unsubscribe";

  params["object"] = "manager_ServerManager";
  params["subscription"] = subscriptionId2;
  params["sessionId"] = sessionId;

  request["params"] = params;

  response = sendRequest (request);

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["retult"]["sessionId"].asString() );

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

  std::cout << "response: " << response.toStyledString() << std::endl;

  BOOST_CHECK (response.isMember ("result") );
  BOOST_CHECK (response["result"].isObject() );
  BOOST_CHECK (response["result"].isMember ("sessionId") );
  BOOST_CHECK (sessionId == response["retult"]["sessionId"].asString() );
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

  check_not_duplicated_event();
}

BOOST_AUTO_TEST_SUITE_END()

} /* kurento */
