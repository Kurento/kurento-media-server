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

#include "httpendpointserver.hpp"

namespace kurento
{

static void
url_removed_cb (KmsHttpEPServer *server, const gchar *uri, gpointer data)
{
  HttpEndPoint *httpEp = (HttpEndPoint *) data;
  std::string uriStr = uri;

  GST_DEBUG ("Remove URI %s", uriStr.c_str() );

  if (httpEp->getUrl().size() <= uriStr.size() )
    return;

  /* Remove the initial "http://host:port" to compare the uri */
  std::string substr = httpEp->getUrl().substr (httpEp->getUrl().size() -
      uriStr.size(), std::string::npos);

  if (substr.compare (uriStr) != 0)
    return;

  GST_INFO ("Launch signal to java application");
}

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

  urlRemovedHandlerId = g_signal_connect (httpepserver, "url-removed",
      G_CALLBACK (url_removed_cb), this);
}

HttpEndPoint::~HttpEndPoint() throw ()
{
  g_signal_handler_disconnect (httpepserver, urlRemovedHandlerId);

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

void
HttpEndPoint::setUrl (std::string newUrl)
{
  url = newUrl;
}

} // kurento
