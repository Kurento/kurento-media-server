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

#include "mediaEvents_types.h"
#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

#define GST_CAT_DEFAULT kurento_http_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHttpEndPoint"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

void
http_end_point_raise_petition_event (HttpEndPoint *httpEp)
{
  MediaEvent event;
  HttpEndPointEvent httpEndPointEvent;
  int method;

  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);

  g_object_get (G_OBJECT (httpEp->element), "http-method", &method, NULL);

  GST_DEBUG ("method: %d", method);

  if (method == 0)
    httpEndPointEvent.__set_request (HttpEndPointRequestEvent::type::GET_REQUEST_EVENT);
  else if (method == 1)
    httpEndPointEvent.__set_request (HttpEndPointRequestEvent::type::POST_REQUEST_EVENT);
  else
    httpEndPointEvent.__set_request (HttpEndPointRequestEvent::type::UNEXPECTED_REQUEST_EVENT);

  httpEndPointEvent.write (&protocol);
  std::string event_str;
  transport->appendBufferToString (event_str);
  event.__set_event (event_str);
  event.__set_source (*httpEp);

  std::dynamic_pointer_cast<MediaPipeline> (httpEp->parent)->sendEvent (event);
  GST_INFO ("Signal raised");
}

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

  http_end_point_raise_petition_event (httpEp);
}

HttpEndPoint::HttpEndPoint (std::shared_ptr<MediaPipeline> parent) :
  EndPoint (parent)
{
  this->type.__set_endPoint (EndPointType::type::HTTP_END_POINT);

  element = gst_element_factory_make ("httpendpoint", NULL);

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

HttpEndPoint::StaticConstructor HttpEndPoint::staticConstructor;

HttpEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
