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

#include "UriEndpointImpl.hpp"

#define GST_CAT_DEFAULT kurento_uri_endpoint_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoUriEndpointImpl"

namespace kurento
{

typedef enum {
  KMS_URI_END_POINT_STATE_STOP,
  KMS_URI_END_POINT_STATE_START,
  KMS_URI_END_POINT_STATE_PAUSE
} KmsUriEndPointState;

UriEndpointImpl::UriEndpointImpl (const std::string &uri,
                                  const std::string &factoryName,
                                  std::shared_ptr< MediaObjectImpl > parent,
                                  int garbagePeriod) :
  EndpointImpl (factoryName, parent,
                garbagePeriod)
{
  g_object_set (G_OBJECT (getGstreamerElement() ), "uri", uri.c_str(), NULL);
}

std::string
UriEndpointImpl::getUri ()
{
  char *uri_str;

  g_object_get (G_OBJECT (getGstreamerElement() ), "uri", &uri_str, NULL);
  std::string uri (uri_str);
  g_free (uri_str);

  return uri;
}

void
UriEndpointImpl::start ()
{
  g_object_set (G_OBJECT (getGstreamerElement() ), "state",
                KMS_URI_END_POINT_STATE_START, NULL);
}

void
UriEndpointImpl::pause ()
{
  g_object_set (G_OBJECT (getGstreamerElement() ), "state",
                KMS_URI_END_POINT_STATE_PAUSE, NULL);
}

void
UriEndpointImpl::stop ()
{
  g_object_set (G_OBJECT (getGstreamerElement() ), "state",
                KMS_URI_END_POINT_STATE_STOP, NULL);
}

UriEndpointImpl::StaticConstructor UriEndpointImpl::staticConstructor;

UriEndpointImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
