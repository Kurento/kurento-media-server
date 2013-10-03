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

#include "HttpEndPoint.hpp"

#include "httpendpointserver.hpp"

#include "HttpEndPointType_constants.h"

// TODO: reuse when needed
#if 0
#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"
#endif

#define GST_CAT_DEFAULT kurento_http_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHttpEndPoint"

// TODO: reuse when needed
#if 0
using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;
#endif

namespace kurento
{

void
http_end_point_raise_petition_event (HttpEndPoint *httpEp, KmsHttpEndPointAction action)
{
// TODO: reuse when needed
#if 0
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
#endif
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

static void
url_removed_cb (KmsHttpEPServer *server, const gchar *uri, gpointer data)
{
  GST_DEBUG ("TODO: Implement url_removed_cb");
}

static void
url_expired_cb (KmsHttpEPServer *server, const gchar *uri, gpointer data)
{
  GST_DEBUG ("TODO: Implement url_expired_cb");
}

HttpEndPoint::HttpEndPoint (std::shared_ptr<MediaPipeline> parent)
  : EndPoint (parent, g_HttpEndPointType_constants.TYPE_NAME)
{
  element = gst_element_factory_make ("httpendpoint", NULL);

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);

  actionRequestedHandlerId = g_signal_connect (httpepserver, "action-requested",
      G_CALLBACK (action_requested_cb), this);
  urlRemovedHandlerId = g_signal_connect (httpepserver, "url-removed",
      G_CALLBACK (url_removed_cb), this);
  urlExpiredHandlerId = g_signal_connect (httpepserver, "url-expired",
      G_CALLBACK (url_expired_cb), this);
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
  g_signal_handler_disconnect (httpepserver, urlExpiredHandlerId);
  g_signal_handler_disconnect (httpepserver, urlRemovedHandlerId);

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
