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

#define BOOST_TEST_MODULE utils_test
#include <boost/test/unit_test.hpp>

#include "utils/marshalling.hpp"

using namespace kurento;

BOOST_AUTO_TEST_SUITE (utils_test)

BOOST_AUTO_TEST_CASE ( marshal_unmarshal_string )
{
  std::string originalStr = "abcd1234";
  std::string marshalledStr, unmarshalledStr;

  marshalledStr = marshalString (originalStr);
  unmarshalledStr = unmarshalString (marshalledStr);

  BOOST_REQUIRE_EQUAL (0, originalStr.compare (unmarshalledStr) );
}

BOOST_AUTO_TEST_SUITE_END ()
