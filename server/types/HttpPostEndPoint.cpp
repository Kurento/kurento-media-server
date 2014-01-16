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

#include "HttpPostEndPoint.hpp"
#include "KmsMediaHttpPostEndPointType_constants.h"

#include "utils/utils.hpp"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_http_post_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHttpPostEndPoint"

namespace kurento
{

HttpPostEndPoint::HttpPostEndPoint (MediaSet &mediaSet,
                                    std::shared_ptr<MediaPipeline> parent,
                                    const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : HttpEndPoint (mediaSet, parent,
                  g_KmsMediaHttpPostEndPointType_constants.TYPE_NAME,
                  params)
{
  /* Do not accept EOS */
  g_object_set ( G_OBJECT (element), "accept-eos", false, NULL);

  register_end_point();

  if (!is_registered() ) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.HTTP_END_POINT_REGISTRATION_ERROR,
                                   "Cannot register HttpGetEndPoint");
    throw except;
  }
}

HttpPostEndPoint::~HttpPostEndPoint() throw ()
{
}

HttpPostEndPoint::StaticConstructor HttpPostEndPoint::staticConstructor;

HttpPostEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento