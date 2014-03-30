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
    return "";
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

static void
register_end_point_adaptor_function (KmsHttpEPServer *self, const gchar *uri,
                                     GError *err, gpointer data)
{
  auto handler =
    reinterpret_cast<std::function<void (const gchar *uri, GError *err) >*>
    (data);

  (*handler) (uri, err);
}

static void
unregister_end_point_adaptor_function (KmsHttpEPServer *self, GError *err,
                                       gpointer data)
{
  auto handler = reinterpret_cast<std::function<void (GError *err) >*> (data);
  (*handler) (err);
}

void
HttpEndpointImpl::unregister_end_point ()
{
  std::string uri = getUriFromUrl (url);
  Glib::Cond cond;
  Glib::Mutex mutex;
  bool finish = FALSE;

  if (!urlSet) {
    return;
  }

  std::function <void (GError *err) > aux = [&] (GError * err) {
    if (err != NULL) {
      GST_ERROR ("Could not unregister uri %s: %s", uri.c_str(), err->message);
    }

    url = "";
    urlSet = false;

    mutex.lock ();
    finish = TRUE;
    cond.signal();
    mutex.unlock ();
  };

  kms_http_ep_server_unregister_end_point (httpepserver, uri.c_str(),
      unregister_end_point_adaptor_function, &aux, NULL);

  mutex.lock ();

  while (!finish) {
    cond.wait (mutex);
  }

  mutex.unlock ();
}

void
HttpEndpointImpl::register_end_point ()
{
  Glib::Mutex mutex;
  Glib::Cond cond;
  bool done = FALSE;

  std::function <void (const gchar *, GError *err) > aux = [&] (const gchar * uri,
  GError * err) {
    gchar *addr;
    guint port;

    if (err != NULL) {
      GST_ERROR ("Can not register end point: %s", err->message);
      goto do_signal;
    }

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

    g_object_get (G_OBJECT (httpepserver), "announced-address", &addr, "port",
                  &port, NULL);

    url = g_strdup_printf ("http://%s:%d%s", addr, port, uri);
    urlSet = true;

do_signal:
    mutex.lock ();
    done = TRUE;
    cond.signal();
    mutex.unlock ();
  };

  kms_http_ep_server_register_end_point (httpepserver, element,
                                         disconnectionTimeout, register_end_point_adaptor_function,
                                         &aux, NULL);
  mutex.lock ();

  while (!done) {
    cond.wait (mutex);
  }

  mutex.unlock ();
}

bool
HttpEndpointImpl::is_registered()
{
  return urlSet;
}


HttpEndpointImpl::HttpEndpointImpl (int disconnectionTimeout,
                                    std::shared_ptr< MediaObjectImpl > parent) :
  SessionEndpointImpl (FACTORY_NAME, parent)
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

      try {
        Error error (shared_from_this(), "Invalid Uri", 0, "INVALID_URI");

        GST_ERROR ("%s", errorMessage.c_str() );

        signalError (error);
      } catch (std::bad_weak_ptr &e) {
      }
    } else {
      try {
        MediaSessionStarted event (shared_from_this(),
                                   MediaSessionStarted::getName() );

        signalMediaSessionStarted (event);
      } catch (std::bad_weak_ptr &e) {
      }
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

    if (actionRequestedHandlerId > 0) {
      g_signal_handler_disconnect (httpepserver, actionRequestedHandlerId);
      actionRequestedHandlerId = 0;
    }

    if (urlExpiredHandlerId > 0) {
      g_signal_handler_disconnect (httpepserver, urlExpiredHandlerId);
      urlExpiredHandlerId = 0;
    }

    if (urlRemovedHandlerId > 0) {
      g_signal_handler_disconnect (httpepserver, urlRemovedHandlerId);
      urlRemovedHandlerId = 0;
    }

    unregister_end_point ();

    if (!g_atomic_int_compare_and_exchange (& (sessionStarted), 1, 0) ) {
      return;
    }

    try {
      MediaSessionTerminated event (shared_from_this(),
                                    MediaSessionTerminated::getName() );

      signalMediaSessionTerminated (event);
    } catch (std::bad_weak_ptr &e) {
    }
  };
}

HttpEndpointImpl::~HttpEndpointImpl()
{
  if (actionRequestedHandlerId > 0) {
    g_signal_handler_disconnect (httpepserver, actionRequestedHandlerId);
  }

  if (urlExpiredHandlerId > 0) {
    g_signal_handler_disconnect (httpepserver, urlExpiredHandlerId);
  }

  if (urlRemovedHandlerId > 0) {
    g_signal_handler_disconnect (httpepserver, urlRemovedHandlerId);
  }

  unregister_end_point ();
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
