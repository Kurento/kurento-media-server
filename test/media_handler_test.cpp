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

#define BOOST_TEST_MODULE media_handler_test
#include <boost/test/unit_test.hpp>

#include "types/MediaHandler.hpp"

using namespace kurento;

BOOST_AUTO_TEST_SUITE (media_handler_test)

BOOST_AUTO_TEST_CASE ( add_remove_media_handlers )
{
  int i, j, n;
  const int nMediaHandlersPerEventType = 20;
  const std::string eventTypes[] = {"a", "b", "c", "d"};
  MediaHandlerManager mediaHandlerManager;
  std::string callbackToken;
  std::set<std::string> callbackTokens;

  n = 0;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < nMediaHandlersPerEventType; j++) {
      n++;
      mediaHandlerManager.addMediaHandler (callbackToken, eventTypes[i], "", n);
      callbackTokens.insert (callbackToken);

      BOOST_REQUIRE_EQUAL (n, mediaHandlerManager.getHandlersMapSize() );
      BOOST_REQUIRE_EQUAL (i + 1, mediaHandlerManager.getEventTypesMapSize() );
      BOOST_REQUIRE_EQUAL (j + 1,
                           mediaHandlerManager.getMediaHandlersSetSize (eventTypes[i]) );
    }
  }

  for (i = 0; i < 4; i++) {
    for (j = 0; j < nMediaHandlersPerEventType; j++) {
      n--;
      callbackToken = * (callbackTokens.begin () );
      mediaHandlerManager.removeMediaHandler (callbackToken);
      callbackTokens.erase (callbackToken);

      BOOST_REQUIRE_EQUAL (n, mediaHandlerManager.getHandlersMapSize() );
    }
  }

  BOOST_REQUIRE_EQUAL (4, mediaHandlerManager.getEventTypesMapSize() );

  for (i = 0; i < 4; i++) {
    BOOST_REQUIRE_EQUAL (0, mediaHandlerManager.getMediaHandlersSetSize (
                           eventTypes[i]) );
  }
}

BOOST_AUTO_TEST_SUITE_END ()
