/*
 * UriEndPoint.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UriEndPoint.hpp"
#include <gst/gst.h>

#define GST_CAT_DEFAULT uri_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "UriEndPoint"

namespace kurento
{

UriEndPoint::UriEndPoint (std::shared_ptr<MediaManager> parent, const std::string &uri) : EndPoint (parent)
{
  this->uri = uri;
}

UriEndPoint::~UriEndPoint() throw ()
{

}

std::string
UriEndPoint::getUri ()
{
  return uri;
}

void
UriEndPoint::start ()
{
  GST_INFO ("URIENDPOINT START");
}

void
UriEndPoint::pause ()
{
  GST_INFO ("URIENDPOINT PAUSE");
}

void
UriEndPoint::stop ()
{
  GST_INFO ("URIENDPOINT STOP");
}

UriEndPoint::StaticConstructor UriEndPoint::staticConstructor;

UriEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
