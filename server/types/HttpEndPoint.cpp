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

#include "KmsMediaHttpEndPointType_constants.h"
#include "KmsMediaHttpEndPointType_types.h"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"
#include "KmsMediaSessionEndPointType_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

#define GST_CAT_DEFAULT kurento_http_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoHttpEndPoint"

#define DISCONNECTION_TIMEOUT 2 /* seconds */
#define REGISTER_TIMEOUT 3 /* seconds */
#define TERMINATE_ON_EOS_DEFAULT false;

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

void
http_end_point_raise_petition_event (HttpEndPoint *httpEp, KmsHttpEndPointAction action)
{
  if (!g_atomic_int_compare_and_exchange (& (httpEp->sessionStarted), 0, 1) )
    return;

  if (action == KMS_HTTP_END_POINT_ACTION_UNDEFINED) {
    GST_ERROR ("Invalid or unexpected request received");
    // TODO: Raise error to remote client
    return;
  }

  httpEp->sendEvent (
    g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_START);
}

static void
action_requested_cb (KmsHttpEPServer *server, const gchar *uri,
                     KmsHttpEndPointAction action, gpointer data)
{
  HttpEndPoint *httpEp = (HttpEndPoint *) data;
  std::string uriStr = uri;
  std::string url = httpEp->getUrl();

  GST_DEBUG ("Action requested URI %s", uriStr.c_str() );

  if (url.size() <= uriStr.size() )
    return;

  /* Remove the initial "http://host:port" to compare the uri */
  std::string substr = url.substr (url.size() - uriStr.size(),
                                   std::string::npos);

  if (substr.compare (uriStr) != 0)
    return;

  http_end_point_raise_petition_event (httpEp, action);
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

void
kurento_http_end_point_raise_session_terminated_event (HttpEndPoint *httpEp, const gchar *uri)
{
  std::string uriStr = uri;

  if (httpEp->url.size() <= uriStr.size() )
    return;

  /* Remove the initial "http://host:port" to compare the uri */
  std::string substr = httpEp->url.substr (httpEp->url.size() - uriStr.size(),
                       std::string::npos);

  if (substr.compare (uriStr) != 0)
    return;

  GST_DEBUG ("Session terminated URI %s", uriStr.c_str() );

  if (!g_atomic_int_compare_and_exchange (& (httpEp->sessionStarted), 1, 0) )
    return;

  httpEp->sendEvent (
    g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_COMPLETE);
}

static void
url_removed_cb (KmsHttpEPServer *server, const gchar *uri, gpointer data)
{
  kurento_http_end_point_raise_session_terminated_event ( (HttpEndPoint *) data,
      uri);
}

static void
url_expired_cb (KmsHttpEPServer *server, const gchar *uri, gpointer data)
{
  kurento_http_end_point_raise_session_terminated_event ( (HttpEndPoint *) data,
      uri);
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
  struct MainLoopData *mdata = (struct MainLoopData *) data;

  return mdata->func (mdata->data);
}

static void
main_loop_data_destroy (gpointer data)
{
  struct MainLoopData *mdata = (struct MainLoopData *) data;

  if (mdata->destroy != NULL)
    mdata->destroy (mdata->data);

  mdata->mutex->unlock();
  g_slice_free (struct MainLoopData, mdata);
}

static void
operate_in_main_loop_context (GSourceFunc func, gpointer data,
                              GDestroyNotify destroy)
{
  struct MainLoopData *mdata;
  Glib::Mutex mutex;

  mdata = g_slice_new (struct MainLoopData);
  mdata->data = data;
  mdata->func = func;
  mdata->destroy = destroy;
  mdata->mutex = &mutex;

  mutex.lock ();
  g_idle_add_full (G_PRIORITY_HIGH_IDLE, main_loop_wrapper_func, mdata,
                   main_loop_data_destroy);
  mutex.lock ();
}

gboolean
init_http_end_point (gpointer data)
{
  HttpEndPoint *httpEp = (HttpEndPoint *) data;
  std::string uri;
  const gchar *url;
  gchar *c_uri;
  gchar *addr;
  guint port;

  httpEp->actionRequestedHandlerId = g_signal_connect (httpepserver, "action-requested",
                                     G_CALLBACK (action_requested_cb), httpEp);
  httpEp->urlRemovedHandlerId = g_signal_connect (httpepserver, "url-removed",
                                G_CALLBACK (url_removed_cb), httpEp);
  httpEp->urlExpiredHandlerId = g_signal_connect (httpepserver, "url-expired",
                                G_CALLBACK (url_expired_cb), httpEp);

  url = kms_http_ep_server_register_end_point (httpepserver, httpEp->element,
        httpEp->disconnectionTimeout);

  if (url == NULL)
    return FALSE;

  g_object_get (G_OBJECT (httpepserver), "announced-address", &addr, "port", &port,
                NULL);
  c_uri = g_strdup_printf ("http://%s:%d%s", addr, port, url);
  uri = c_uri;

  g_free (addr);
  g_free (c_uri);

  httpEp->setUrl (uri);
  httpEp->urlSet = true;

  return FALSE;
}

gboolean
dispose_http_end_point (gpointer data)
{
  HttpEndPoint *httpEp = (HttpEndPoint *) data;

  g_signal_handler_disconnect (httpepserver, httpEp->actionRequestedHandlerId);
  g_signal_handler_disconnect (httpepserver, httpEp->urlExpiredHandlerId);
  g_signal_handler_disconnect (httpepserver, httpEp->urlRemovedHandlerId);

  std::string uri = getUriFromUrl (httpEp->url);

  if (!uri.empty() )
    kms_http_ep_server_unregister_end_point (httpepserver, uri.c_str() );

  return FALSE;
}

void
kurento_http_end_point_eos_detected_cb (GstElement *element, gpointer data)
{
  HttpEndPoint *httpEp = (HttpEndPoint *) data;
  httpEp->sendEvent (
    g_KmsMediaHttpEndPointType_constants.EVENT_EOS_DETECTED);
}

void
HttpEndPoint::init (std::shared_ptr<MediaPipeline> parent,
                    guint disconnectionTimeout, bool terminateOnEOS,
                    KmsMediaProfile profile)
throw (KmsMediaServerException)
{
  element = gst_element_factory_make ("httpendpoint", NULL);

  g_object_set ( G_OBJECT (element), "is-live", !terminateOnEOS, NULL);

  switch (profile.mediaMuxer) {
  case KmsMediaMuxer::WEBM:
    g_object_set ( G_OBJECT (element), "profile", "webm", NULL);
    GST_INFO ("Set WEBM profile");
    break;

  case KmsMediaMuxer::MP4:
    g_object_set ( G_OBJECT (element), "profile", "mp4", NULL);
    GST_INFO ("Set MP4 profile");
    break;
  }

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);

  g_signal_connect (element, "eos-detected",
                    G_CALLBACK (kurento_http_end_point_eos_detected_cb), this);

  this->disconnectionTimeout = disconnectionTimeout;

  operate_in_main_loop_context (init_http_end_point, this, NULL);

  if (!urlSet) {
    KmsMediaServerException except;

    createKmsMediaServerException (except, g_KmsMediaErrorCodes_constants.HTTP_END_POINT_REGISTRATION_ERROR,
                                   "Cannot register HttpEndPoint");
    throw except;
  }
}

KmsMediaHttpEndPointConstructorParams
unmarshalKmsMediaHttpEndPointConstructorParams (std::string data)
throw (KmsMediaServerException)
{
  KmsMediaHttpEndPointConstructorParams params;
  boost::shared_ptr<TMemoryBuffer> transport;

  try {
    transport = boost::shared_ptr<TMemoryBuffer> (new TMemoryBuffer ( (uint8_t *) data.data(), data.size () ) );
    TBinaryProtocol protocol = TBinaryProtocol (transport);
    params.read (&protocol);
  } catch (...) {
    KmsMediaServerException except;

    createKmsMediaServerException (except, g_KmsMediaErrorCodes_constants.UNMARSHALL_ERROR,
                                   "Cannot unmarshal KmsMediaHttpEndPointConstructorParams");
    throw except;
  }

  return params;
}

HttpEndPoint::HttpEndPoint (MediaSet &mediaSet,
                            std::shared_ptr<MediaPipeline> parent,
                            const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : EndPoint (mediaSet, parent, g_KmsMediaHttpEndPointType_constants.TYPE_NAME,
              params)
{
  const KmsMediaParam *p;
  KmsMediaHttpEndPointConstructorParams httpEpParams;
  guint disconnectionTimeout = DISCONNECTION_TIMEOUT;
  bool terminateOnEOS = TERMINATE_ON_EOS_DEFAULT;
  KmsMediaProfile profile;

  profile.mediaMuxer = KmsMediaMuxer::WEBM;

  p = getParam (params, g_KmsMediaHttpEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);

  if (p != NULL) {
    httpEpParams = unmarshalKmsMediaHttpEndPointConstructorParams (p->data);

    if (httpEpParams.__isset.disconnectionTimeout) {
      disconnectionTimeout = httpEpParams.disconnectionTimeout;
    }

    if (httpEpParams.__isset.terminateOnEOS)
      terminateOnEOS = httpEpParams.terminateOnEOS;

    if (httpEpParams.__isset.profileType)
      profile = httpEpParams.profileType;
  }

  init (parent, disconnectionTimeout, terminateOnEOS, profile);
}

HttpEndPoint::~HttpEndPoint() throw ()
{
  operate_in_main_loop_context (dispose_http_end_point, this, NULL);

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
HttpEndPoint::setUrl (const std::string &newUrl)
{
  url = newUrl;
}

void
HttpEndPoint::invoke (KmsMediaInvocationReturn &_return,
                      const std::string &command,
                      const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaHttpEndPointType_constants.GET_URL.compare (command) == 0)
    createStringInvocationReturn (_return, url);
  else
    EndPoint::invoke (_return, command, params);
}

void
HttpEndPoint::subscribe (std::string &_return, const std::string &eventType, const std::string &handlerAddress, const int32_t handlerPort)
throw (KmsMediaServerException)
{
  if (g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_START.compare (
        eventType) == 0)
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  else if (g_KmsMediaSessionEndPointType_constants.EVENT_MEDIA_SESSION_COMPLETE.compare (
             eventType) == 0)
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  else if (g_KmsMediaHttpEndPointType_constants.EVENT_EOS_DETECTED.compare (
             eventType) == 0)
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  else
    EndPoint::subscribe (_return, eventType, handlerAddress, handlerPort);
}

HttpEndPoint::StaticConstructor HttpEndPoint::staticConstructor;

HttpEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
