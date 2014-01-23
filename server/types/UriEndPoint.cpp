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

#include "UriEndPoint.hpp"

#include "KmsMediaUriEndPointType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

#define GST_CAT_DEFAULT kurento_uri_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoUriEndPoint"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

typedef enum {
  KMS_URI_END_POINT_STATE_STOP,
  KMS_URI_END_POINT_STATE_START,
  KMS_URI_END_POINT_STATE_PAUSE
} KmsUriEndPointState;

UriEndPoint::UriEndPoint (MediaSet &mediaSet,
                          std::shared_ptr<MediaPipeline> parent,
                          const std::string &type,
                          const std::map<std::string, KmsMediaParam> &params,
                          const std::string &factoryName)
throw (KmsMediaServerException)
  : EndPoint (mediaSet, parent, type, params, factoryName)
{
  KmsMediaUriEndPointConstructorParams uriEpParams;
  const KmsMediaParam *p;

  p = getParam (params,
                g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);

  if (p == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR,
                                   "Param '" + g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE +
                                   "' not found");
    throw except;
  }

  unmarshalKmsMediaUriEndPointConstructorParams (uriEpParams, p->data);

  g_object_set (G_OBJECT (element), "uri", uriEpParams.uri.c_str(), NULL);
}

UriEndPoint::~UriEndPoint() throw ()
{

}

std::string
UriEndPoint::getUri ()
{
  char *uri_str;

  g_object_get (G_OBJECT (element), "uri", &uri_str, NULL);
  std::string uri (uri_str);
  g_free (uri_str);

  return uri;
}

void
UriEndPoint::start ()
{
  g_object_set (G_OBJECT (element), "state", KMS_URI_END_POINT_STATE_START, NULL);
}

void
UriEndPoint::pause ()
{
  g_object_set (G_OBJECT (element), "state", KMS_URI_END_POINT_STATE_PAUSE, NULL);
}

void
UriEndPoint::stop ()
{
  g_object_set (G_OBJECT (element), "state", KMS_URI_END_POINT_STATE_STOP, NULL);
}

void
UriEndPoint::invoke (KmsMediaInvocationReturn &_return,
                     const std::string &command,
                     const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaUriEndPointType_constants.GET_URI.compare (command) == 0) {
    createStringInvocationReturn (_return, getUri () );
  } else if (g_KmsMediaUriEndPointType_constants.START.compare (command) == 0) {
    start ();
    createVoidInvocationReturn (_return);
  } else if (g_KmsMediaUriEndPointType_constants.PAUSE.compare (command) == 0) {
    pause ();
    createVoidInvocationReturn (_return);
  } else if (g_KmsMediaUriEndPointType_constants.STOP.compare (command) == 0) {
    stop ();
    createVoidInvocationReturn (_return);
  } else {
    EndPoint::invoke (_return, command, params);
  }
}

UriEndPoint::StaticConstructor UriEndPoint::staticConstructor;

UriEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
