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

#include "HttpEndpointImpl.hpp"

#define GST_CAT_DEFAULT kurento_http_endpoint_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHttpEndpointImpl"

#define FACTORY_NAME "httpendpoint"

namespace kurento
{

static void
action_requested_adaptor_function (KmsHttpEPServer *server, const gchar *uri,
                                   KmsHttpEndPointAction action, gpointer data)
{
  auto handler =
    reinterpret_cast < std::function < void (const gchar * uri,
        KmsHttpEndPointAction action) > * >
    (data);

  (*handler) (uri, action);
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
      } else {
        continue;
      }
    }

    if (c != '/') {
      continue;
    }

    uri = url.substr (i, std::string::npos);
    break;
  }

  return uri;
}

static void
session_terminated_adaptor_function (KmsHttpEPServer *server, const gchar *uri,
                                     gpointer data)
{
  auto handler = reinterpret_cast<std::function<void (const gchar *uri) >*>
                 (data);

  (*handler) (uri);
}

struct MainLoopData {
  gpointer data;
  GSourceFunc func;
  GDestroyNotify destroy;
  Glib::Mutex *mutex;
};

static gboolean
main_loop_wrapper_func (gpointer data)
{
  auto func = reinterpret_cast<std::function<void() >*> (data);

  (*func) ();

  return G_SOURCE_REMOVE;
}

static void
operate_in_main_loop_context (std::function<void () > &func)
{
  Glib::Mutex mutex;

  std::function <void () > handler = [&] () {
    func ();
    mutex.unlock();
  };

  mutex.lock ();
  g_idle_add_full (G_PRIORITY_HIGH_IDLE, main_loop_wrapper_func, &handler,
                   NULL);
  mutex.lock ();
}

void
HttpEndpointImpl::register_end_point ()
{
  std::function<void() > initEndPoint = [&] () {
    std::string uri;
    const gchar *url;
    gchar *c_uri;
    gchar *addr;
    guint port;

    actionRequestedHandlerId = g_signal_connect (httpepserver,
                               "action-requested",
                               G_CALLBACK (action_requested_adaptor_function),
                               &actionRequestedLambda);
    urlRemovedHandlerId = g_signal_connect (httpepserver, "url-removed",
                                            G_CALLBACK (session_terminated_adaptor_function),
                                            &sessionTerminatedLambda);
    urlExpiredHandlerId = g_signal_connect (httpepserver, "url-expired",
                                            G_CALLBACK (session_terminated_adaptor_function),
                                            &sessionTerminatedLambda);

    url = kms_http_ep_server_register_end_point (httpepserver, element,
          disconnectionTimeout);

    if (url == NULL) {
      return;
    }

    g_object_get (G_OBJECT (httpepserver), "announced-address", &addr, "port",
                  &port,
                  NULL);
    c_uri = g_strdup_printf ("http://%s:%d%s", addr, port, url);
    uri = c_uri;

    g_free (addr);
    g_free (c_uri);

    this->url = uri;
    urlSet = true;

    return;
  };

  operate_in_main_loop_context (initEndPoint);
}

bool
HttpEndpointImpl::is_registered()
{
  return urlSet;
}


HttpEndpointImpl::HttpEndpointImpl (int disconnectionTimeout,
                                    std::shared_ptr< MediaObjectImpl > parent,
                                    int garbagePeriod) :
  SessionEndpointImpl (FACTORY_NAME, parent, garbagePeriod)
{
  this->disconnectionTimeout = disconnectionTimeout;

  actionRequestedLambda = [&] (const gchar * uri,
  KmsHttpEndPointAction action) {
    std::string uriStr = uri;

    GST_DEBUG ("Action requested URI %s", uriStr.c_str() );

    if (url.size() <= uriStr.size() ) {
      return;
    }

    /* Remove the initial "http://host:port" to compare the uri */
    std::string substr = url.substr (url.size() - uriStr.size(),
                                     std::string::npos);

    if (substr.compare (uriStr) != 0) {
      return;
    }

    /* Send event */
    if (!g_atomic_int_compare_and_exchange (& (sessionStarted), 0, 1) ) {
      return;
    }

    if (action == KMS_HTTP_END_POINT_ACTION_UNDEFINED) {
      std::string errorMessage = "Invalid or unexpected request received";
      Error error (shared_from_this(), "Invalid Uri", 0, "INVALID_URI");

      GST_ERROR ("%s", errorMessage.c_str() );

      signalError (error);
    } else {
      MediaSessionStarted event (shared_from_this(),
                                 MediaSessionStarted::getName() );

      signalMediaSessionStarted (event);
    }
  };

  sessionTerminatedLambda = [&] (const gchar * uri) {
    std::string uriStr = uri;

    if (url.size() <= uriStr.size() ) {
      return;
    }

    /* Remove the initial "http://host:port" to compare the uri */
    std::string substr = url.substr (url.size() - uriStr.size(),
                                     std::string::npos);

    if (substr.compare (uriStr) != 0) {
      return;
    }

    GST_DEBUG ("Session terminated URI %s", uriStr.c_str() );

    if (!g_atomic_int_compare_and_exchange (& (sessionStarted), 1, 0) ) {
      return;
    }

    MediaSessionTerminated event (shared_from_this(),
                                  MediaSessionTerminated::getName() );

    signalMediaSessionTerminated (event);
  };
}

HttpEndpointImpl::~HttpEndpointImpl()
{
  std::function <void() > aux = [&] () {
    g_signal_handler_disconnect (httpepserver, actionRequestedHandlerId);
    g_signal_handler_disconnect (httpepserver, urlExpiredHandlerId);
    g_signal_handler_disconnect (httpepserver, urlRemovedHandlerId);

    std::string uri = getUriFromUrl (url);

    if (!uri.empty() ) {
      kms_http_ep_server_unregister_end_point (httpepserver, uri.c_str() );
    }
  };

  operate_in_main_loop_context (aux);
}

std::string
HttpEndpointImpl::getUrl ()
{
  return url;
}

HttpEndpointImpl::StaticConstructor HttpEndpointImpl::staticConstructor;

HttpEndpointImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
