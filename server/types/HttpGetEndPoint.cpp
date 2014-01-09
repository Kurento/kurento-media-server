/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#include "HttpGetEndPoint.hpp"
#include "KmsMediaHttpGetEndPointType_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#define TERMINATE_ON_EOS_DEFAULT false;

#define GST_CAT_DEFAULT kurento_http_get_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHttpGetEndPoint"

namespace kurento
{

void
HttpGetEndPoint::subscribe (std::string &_return, const std::string &eventType,
                            const std::string &handlerAddress,
                            const int32_t handlerPort)
throw (KmsMediaServerException)
{
  if (g_KmsMediaHttpGetEndPointType_constants.EVENT_EOS_DETECTED.compare (
        eventType) == 0)
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  else
    HttpEndPoint::subscribe (_return, eventType, handlerAddress, handlerPort);
}

void
HttpGetEndPoint::init (bool terminateOnEOS, KmsMediaProfile profile)
throw (KmsMediaServerException)
{
  g_object_set ( G_OBJECT (element), "accept-eos", terminateOnEOS, NULL);
  g_object_set ( G_OBJECT (element), "profile", profile.mediaMuxer, NULL);

  switch (profile.mediaMuxer) {
  case KmsMediaMuxer::WEBM:
    GST_INFO ("Set WEBM profile");
    break;

  case KmsMediaMuxer::MP4:
    GST_INFO ("Set MP4 profile");
    break;
  }
}

KmsMediaHttpGetEndPointConstructorParams
unmarshalKmsMediaHttpGetEndPointConstructorParams (std::string data)
throw (KmsMediaServerException)
{
  KmsMediaHttpGetEndPointConstructorParams params;
  boost::shared_ptr<TMemoryBuffer> transport;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    params.read (&protocol);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except, g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Cannot unmarshal KmsMediaHttpGetEndPointConstructorParams");
    throw except;
  }

  return params;
}

HttpGetEndPoint::HttpGetEndPoint (MediaSet &mediaSet,
                                  std::shared_ptr<MediaPipeline> parent,
                                  const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : HttpEndPoint (mediaSet, parent, g_KmsMediaHttpGetEndPointType_constants.TYPE_NAME,
                  params)
{
  const KmsMediaParam *p;
  KmsMediaHttpGetEndPointConstructorParams httpGetEpParams;
  bool terminateOnEOS = TERMINATE_ON_EOS_DEFAULT;
  KmsMediaProfile profile;

  profile.mediaMuxer = KmsMediaMuxer::WEBM;

  p = getParam (params, g_KmsMediaHttpGetEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);

  if (p != NULL) {
    KmsMediaHttpGetEndPointConstructorParams httpGetEpParams;

    httpGetEpParams = unmarshalKmsMediaHttpGetEndPointConstructorParams (p->data);

    if (httpGetEpParams.__isset.terminateOnEOS)
      terminateOnEOS = httpGetEpParams.terminateOnEOS;

    if (httpGetEpParams.__isset.profileType)
      profile = httpGetEpParams.profileType;
  }

  init (terminateOnEOS, profile);
  register_end_point();

  if (!is_registered() ) {
    KmsMediaServerException except;

    createKmsMediaServerException (except, g_KmsMediaErrorCodes_constants.HTTP_END_POINT_REGISTRATION_ERROR,
                                   "Cannot register HttpGetEndPoint");
    throw except;
  }
}

HttpGetEndPoint::~HttpGetEndPoint() throw ()
{
}

HttpGetEndPoint::StaticConstructor HttpGetEndPoint::staticConstructor;

HttpGetEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
