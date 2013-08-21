/*
 * HttpEndPoint.cpp - Kurento Media Server
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

#include "HttpEndPoint.hpp"

namespace kurento
{

HttpEndPoint::HttpEndPoint (std::shared_ptr<MediaPipeline> parent) :
  EndPoint (parent)
{
  gchar *name;
  this->type.__set_endPoint (EndPointType::type::HTTP_END_POINT);

  name = getIdStr ();
  element = gst_element_factory_make ("httpendpoint", name);
  g_free (name);

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
}

HttpEndPoint::~HttpEndPoint() throw ()
{
  gst_bin_remove (GST_BIN ( (
      (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

std::string
HttpEndPoint::getUrl ()
{
  return url;
}

} // kurento
