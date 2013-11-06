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

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#include "KmsMediaUriEndPointType_constants.h"

using namespace kurento;

BOOST_AUTO_TEST_SUITE (utils_test)

BOOST_AUTO_TEST_CASE ( test_marshal_unmarshal_string )
{
  std::string originalStr = "abcd1234";
  std::string marshalledStr, unmarshalledStr;

  marshalString (marshalledStr, originalStr);
  unmarshalString (unmarshalledStr, marshalledStr);

  BOOST_REQUIRE_EQUAL (0, originalStr.compare (unmarshalledStr) );
}

BOOST_AUTO_TEST_CASE ( test_KmsMediaUriEndPointConstructorParams )
{
  const std::string originalStr = "abcd1234";
  std::map<std::string, KmsMediaParam> params;
  const KmsMediaParam *p;
  KmsMediaUriEndPointConstructorParams uriEpParams;

  createKmsMediaUriEndPointConstructorParams (params, originalStr);
  p = getParam (params, g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);
  unmarshalKmsMediaUriEndPointConstructorParams (uriEpParams, p->data);

  BOOST_REQUIRE_EQUAL (0, originalStr.compare (uriEpParams.uri) );
}

BOOST_AUTO_TEST_CASE ( struct_marshal_unmarshal_test )
{
  KmsMediaPointerDetectorWindowSet originalSet, newSet;
  std::string marshalledSet;

  for (int i = 0; i < 5; i++) {
    KmsMediaPointerDetectorWindow aux;
    aux.topRightCornerX = 10;
    aux.topRightCornerY = 40;
    aux.height = 30;
    aux.width = 20;
    aux.id = "test" + i;
    originalSet.windows.insert (aux);
  }

  marshalStruct (marshalledSet, originalSet);
  unmarshalStruct (newSet, marshalledSet);

  for (auto it = newSet.windows.begin(); it != newSet.windows.end(); ++it) {
    if (originalSet.windows.find (*it) == originalSet.windows.end() ) {
      BOOST_FAIL ("Element not found");
    }
  }
}

BOOST_AUTO_TEST_CASE ( struct_marshal_unmarshal_test2 )
{
  KmsMediaPointerDetectorWindow originalStruct, newStruct;
  std::string marshalledStruct;

  originalStruct.topRightCornerX = 10;
  originalStruct.topRightCornerY = 40;
  originalStruct.height = 30;
  originalStruct.width = 20;
  originalStruct.id = "test";

  marshalStruct (marshalledStruct, originalStruct);
  unmarshalStruct (newStruct, marshalledStruct);

  if (originalStruct != newStruct ) {
    BOOST_FAIL ("Test fails");
  }
}

BOOST_AUTO_TEST_SUITE_END ()
