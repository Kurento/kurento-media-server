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
http_end_point_raise_petition_event (HttpEndPoint *httpEp, KmsHttpEndPointAction action)
{
  MediaEvent event;
  HttpEndPointEvent httpEndPointEvent;

  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);

  switch (action) {
  case KMS_HTTP_END_POINT_ACTION_GET:
    httpEndPointEvent.__set_request (HttpEndPointRequestEvent::type::GET_REQUEST_EVENT);
    break;
  case KMS_HTTP_END_POINT_ACTION_POST:
    httpEndPointEvent.__set_request (HttpEndPointRequestEvent::type::POST_REQUEST_EVENT);
    break;
  default:
    httpEndPointEvent.__set_request (HttpEndPointRequestEvent::type::UNEXPECTED_REQUEST_EVENT);
    break;
  }

  httpEndPointEvent.write (&protocol);
  std::string event_str;
  transport->appendBufferToString (event_str);
  event.__set_event (event_str);
  event.__set_source (*httpEp);

  std::dynamic_pointer_cast<MediaPipeline> (httpEp->parent)->sendEvent (event);
  GST_INFO ("Signal raised");
}

static void
action_requested_cb (KmsHttpEPServer *server, const gchar *uri,
    KmsHttpEndPointAction action, gpointer data)
{
  HttpEndPoint *httpEp = (HttpEndPoint *) data;
  std::string uriStr = uri;

  GST_DEBUG ("Action requested URI %s", uriStr.c_str() );

  if (httpEp->getUrl().size() <= uriStr.size() )
    return;

  /* Remove the initial "http://host:port" to compare the uri */
  std::string substr = httpEp->getUrl().substr (httpEp->getUrl().size() -
      uriStr.size(), std::string::npos);

  if (substr.compare (uriStr) != 0)
    return;

  http_end_point_raise_petition_event (httpEp, action);
}

HttpEndPoint::HttpEndPoint (std::shared_ptr<MediaPipeline> parent) :
  EndPoint (parent)
{
  this->type.__set_endPoint (EndPointType::type::HTTP_END_POINT);

  element = gst_element_factory_make ("httpendpoint", NULL);

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);

  actionRequestedHandlerId = g_signal_connect (httpepserver, "action-requested",
      G_CALLBACK (action_requested_cb), this);
}

static std::string
getUriFromUrl (std::string url)
{
  std::string uri;
  gboolean host_read = FALSE;

  /* skip first 7 characters in the url regarding the protocol "http://" */
  if (url.size() < 7) {
    GST_ERROR ("Invalid URL %s", url.c_str() );
    return NULL;
  }

  for ( guint i = 7; i < url.size(); i++) {
    gchar c = url.at (i);

    if (!host_read) {
      if (c == '/') {
        /* URL has no port */
        uri = url.substr (i, std::string::npos);
        break;
      } else if (c == ':') {
        /* skip port number */
        host_read = TRUE;
        continue;
      } else
        continue;
    }

    if (c != '/')
      continue;

    uri = url.substr (i, std::string::npos);
    break;
  }

  return uri;
}

HttpEndPoint::~HttpEndPoint() throw ()
{
  g_signal_handler_disconnect (httpepserver, actionRequestedHandlerId);

  std::string uri = getUriFromUrl (url);

  if (!uri.empty() )
    kms_http_ep_server_unregister_end_point (httpepserver, uri.c_str() );

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
