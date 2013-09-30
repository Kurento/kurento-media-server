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

#define GST_CAT_DEFAULT kurento_uri_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoUriEndPoint"

namespace kurento
{

UriEndPoint::UriEndPoint (std::shared_ptr<MediaPipeline> parent, UriEndPointType::type type) : EndPoint (parent)
{
  this->type.__set_uriEndPoint (type);
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
  g_object_set (G_OBJECT (element), "state", 1 /* start */, NULL);
}

void
UriEndPoint::pause ()
{
  g_object_set (G_OBJECT (element), "state", 2 /* pause */, NULL);
}

void
UriEndPoint::stop ()
{
  g_object_set (G_OBJECT (element), "state", 0 /* stop */, NULL);
}

UriEndPoint::StaticConstructor UriEndPoint::staticConstructor;

UriEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
