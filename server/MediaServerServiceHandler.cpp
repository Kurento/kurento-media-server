/*
 * MediaServerServiceHandler.cpp - Kurento Media Server
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

#include "MediaServerServiceHandler.hpp"
#include "mediaServer_constants.h"
#include <gst/gst.h>

#include "types/MediaPipeline.hpp"
#include "types/MediaElement.hpp"
#include "types/SdpEndPoint.hpp"
#include "types/UriEndPoint.hpp"
#include "types/HttpEndPoint.hpp"
#include "types/Mixer.hpp"
#include "types/Filter.hpp"
#include "types/MediaHandler.hpp"

#define GST_CAT_DEFAULT kurento_media_server_service_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaServerServiceHandler"

#include "httpendpointserver.hpp"

namespace kurento
{

MediaServerServiceHandler::MediaServerServiceHandler ()
{
}

MediaServerServiceHandler::~MediaServerServiceHandler ()
{
}

int32_t
MediaServerServiceHandler::getVersion ()
{
  int32_t v;
  mediaServerConstants *c;

  c = new mediaServerConstants();
  v = c->VERSION;
  delete c;

  return v;
}

void
MediaServerServiceHandler::addHandlerAddress (const int32_t handlerId, const std::string &address,
    const int32_t port) throw (MediaServerException)
{
  std::shared_ptr<MediaHandler> mh;
  std::shared_ptr<MediaHandlerAddress> mha;

  mediaHandlerMutex.lock ();
  mh = mediaHandlerMap.getValue (handlerId);

  if (mh == NULL) {
    mh = std::shared_ptr<MediaHandler> (new MediaHandler (handlerId) );
    mediaHandlerMap.put (handlerId, mh);
  }

  mediaHandlerMutex.unlock ();

  mha = std::shared_ptr<MediaHandlerAddress> (new MediaHandlerAddress (address, port) );
  mh->addAddress (mha);
}

/* MediaObject */

void
MediaServerServiceHandler::release (const MediaObjectId &mediaObject) throw (MediaObjectNotFoundException, MediaServerException)
{
  mediaSet.remove (mediaObject);
}

void
MediaServerServiceHandler::getParent (MediaObjectId &_return, const MediaObjectId &mediaObject)
throw (MediaObjectNotFoundException, NoParentException, MediaServerException)
{
  std::shared_ptr<MediaObjectImpl> mo;
  std::shared_ptr<MediaObjectId> parent;

  try {
    mo = mediaSet.getMediaObject<MediaObjectImpl> (mediaObject);
    parent = mo->getParent ();
    _return = *parent;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (const NoParentException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

/* MediaPipeline */

void
MediaServerServiceHandler::createMediaPipeline (MediaObjectId &_return, const int32_t handlerId)
throw (MediaObjectNotFoundException, HandlerNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaHandler> mh;
  std::shared_ptr<MediaPipeline> mediaPipeline;

  try {
    mh = mediaHandlerMap.getValue (handlerId);

    if (mh == NULL) {
      throw HandlerNotFoundException();
    }

    mediaPipeline = std::shared_ptr<MediaPipeline> (new MediaPipeline (mh) );
    GST_DEBUG ("createMediaPipeline id: %ld, token: %s", mediaPipeline->id, mediaPipeline->token.c_str() );
    mediaSet.put (mediaPipeline);

    _return = *mediaPipeline;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (const HandlerNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::createSdpEndPoint (MediaObjectId &_return, const MediaObjectId &mediaPipeline,
    const SdpEndPointType::type type) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<SdpEndPoint> sdpEp;

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    sdpEp = mm->createSdpEndPoint (type);
    mediaSet.put (sdpEp);

    _return = *sdpEp;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::createSdpEndPointWithFixedSdp (MediaObjectId &_return, const MediaObjectId &mediaPipeline,
    const SdpEndPointType::type type, const std::string &sdp)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<SdpEndPoint> sdpEp;

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    sdpEp = mm->createSdpEndPoint (type, sdp);
    mediaSet.put (sdpEp);

    _return = *sdpEp;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::createUriEndPoint (MediaObjectId &_return, const MediaObjectId &mediaPipeline, const UriEndPointType::type type,
    const std::string &uri) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<UriEndPoint> uriEp;

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    uriEp = mm->createUriEndPoint (type, uri);
    mediaSet.put (uriEp);

    _return = *uriEp;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
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

  mdata->mutex->lock();

  g_idle_add_full (G_PRIORITY_HIGH_IDLE, main_loop_wrapper_func, mdata,
      main_loop_data_destroy);

  mutex.lock();
}

gboolean
register_http_end_point (gpointer data)
{
  HttpEndPoint *httpEp = (HttpEndPoint *) data;
  std::string uri;
  const gchar *url;
  gchar *c_uri;
  gchar *interface;
  guint port;

  url = kms_http_ep_server_register_end_point (httpepserver, httpEp->element, NULL);

  g_object_get (G_OBJECT (httpepserver), "interface", &interface, "port", &port,
      NULL);
  c_uri = g_strdup_printf ("http://%s:%d%s", interface, port, url);
  uri = c_uri;

  g_free (interface);
  g_free (c_uri);

  httpEp->setUrl (uri);
  return FALSE;
}

void
MediaServerServiceHandler::createHttpEndPoint (MediaObjectId &_return, const MediaObjectId &mediaPipeline)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<HttpEndPoint> httpEp;

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    httpEp = mm->createHttpEndPoint ();
    mediaSet.put (httpEp);

    operate_in_main_loop_context (register_http_end_point, &*httpEp, NULL);

    _return = *httpEp;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::createMixer (MediaObjectId &_return, const MediaObjectId &mediaPipeline, const MixerType::type type)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<Mixer> mixer;

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    mixer = mm->createMixer (type);
    mediaSet.put (mixer);

    _return = *mixer;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::createFilter (MediaObjectId &_return, const MediaObjectId &mediaPipeline, const FilterType::type type)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<Filter> filter;

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    filter = mm->createFilter (type);
    mediaSet.put (filter);

    _return = *filter;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

/* MediaElement */

void
MediaServerServiceHandler::sendCommand (CommandResult &_return, const MediaObjectId &mediaElement,
    const Command &command) throw (MediaObjectNotFoundException, EncodingException, MediaServerException)
{
  // TODO: implement
}

void
MediaServerServiceHandler::getMediaSrcs (std::vector<MediaObjectId> & _return, const MediaObjectId &mediaElement)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs = NULL;
  std::vector< std::shared_ptr<MediaSrc> >::iterator it;

  try {
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSrcs = me->getMediaSrcs();

    for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSrcs;
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSrcs;
    throw e;
  } catch (...) {
    delete mediaSrcs;
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::getMediaSinks (std::vector<MediaObjectId> & _return, const MediaObjectId &mediaElement)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks = NULL;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  try {
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSinks = me->getMediaSinks();

    for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSinks;
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSinks;
    throw e;
  } catch (...) {
    delete mediaSinks;
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::getMediaSrcsByMediaType (std::vector<MediaObjectId> & _return, const MediaObjectId &mediaElement,
    const MediaType::type mediaType) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs = NULL;
  std::vector< std::shared_ptr<MediaSrc> >::iterator it;

  try {
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSrcs = me->getMediaSrcsByMediaType (mediaType);

    for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSrcs;
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSrcs;
    throw e;
  } catch (...) {
    delete mediaSrcs;
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::getMediaSinksByMediaType (std::vector<MediaObjectId> & _return, const MediaObjectId &mediaElement,
    const MediaType::type mediaType) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks = NULL;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  try {
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSinks = me->getMediaSinksByMediaType (mediaType);

    for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSinks;
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSinks;
    throw e;
  } catch (...) {
    delete mediaSinks;
    throw MediaServerException();
  }
}

/* MediaPad */

MediaType::type
MediaServerServiceHandler::getMediaType (const MediaObjectId &mediaPad) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPad> pad;

  pad = mediaSet.getMediaObject<MediaPad> (mediaPad);
  return pad->getMediaType ();
}

/* MediaSrc */

void
MediaServerServiceHandler::connect (const MediaObjectId &mediaSrc, const MediaObjectId &mediaSink)
throw (MediaObjectNotFoundException, ConnectionException, MediaServerException)
{
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  try {
    src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
    sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
    src->connect (sink);
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (const ConnectionException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::disconnect (const MediaObjectId &mediaSrc, const MediaObjectId &mediaSink)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  try {
    src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
    sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
    src->disconnect (sink);
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::getConnectedSinks (std::vector < MediaObjectId >&_return, const MediaObjectId &mediaSrc)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaSrc> src;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks = NULL;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  try {
    src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
    mediaSinks = src->getConnectedSinks();

    for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSinks;
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSinks;
    throw e;
  } catch (...) {
    delete mediaSinks;
    throw MediaServerException();
  }
}

/* MediaSink */

void
MediaServerServiceHandler::getConnectedSrc (MediaObjectId &_return, const MediaObjectId &mediaSink)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaSink> sink;

  try {
    sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
    _return = * (sink->getConnectedSrc() );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

/* Mixer */

void
MediaServerServiceHandler::createMixerEndPoint (MediaObjectId &_return, const MediaObjectId &mixer)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<Mixer> m;
  std::shared_ptr<MixerEndPoint> mixerEndPoint;

  try {
    m = mediaSet.getMediaObject<Mixer> (mixer);
    mixerEndPoint = m->createMixerEndPoint();
    mediaSet.put (mixerEndPoint);

    _return = *mixerEndPoint;
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

/* HttpEndPoint */

void
MediaServerServiceHandler::getUrl (std::string &_return, const MediaObjectId &httpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<HttpEndPoint> httpEp;

  try {
    httpEp = mediaSet.getMediaObject<HttpEndPoint> (httpEndPoint);
    _return.assign (httpEp->getUrl () );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

/* UriEndPoint */

void
MediaServerServiceHandler::getUri (std::string &_return, const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  try {
    uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
    _return.assign (uep->getUri() );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::start (const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  try {
    uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
    uep->start();
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::pause (const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  try {
    uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
    uep->pause();
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::stop (const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
  uep->stop();
}

/* SdpEndPoint */

void
MediaServerServiceHandler::generateOffer (std::string &_return, const MediaObjectId &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->generateOffer () );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::processAnswer (std::string &_return, const MediaObjectId &sdpEndPoint, const std::string &answer)
throw (MediaObjectNotFoundException, NegotiationException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->processAnswer (answer) );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (const NegotiationException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::processOffer (std::string &_return, const MediaObjectId &sdpEndPoint, const std::string &offer)
throw (MediaObjectNotFoundException, NegotiationException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->processOffer (offer) );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (const NegotiationException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::getLocalSessionDescription (std::string &_return, const MediaObjectId &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->getLocalSessionDescription () );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

void
MediaServerServiceHandler::getRemoteSessionDescription (std::string &_return, const MediaObjectId &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->getRemoteSessionDescription () );
  } catch (const MediaObjectNotFoundException &e) {
    throw e;
  } catch (...) {
    throw MediaServerException();
  }
}

MediaServerServiceHandler::StaticConstructor MediaServerServiceHandler::staticConstructor;

MediaServerServiceHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
