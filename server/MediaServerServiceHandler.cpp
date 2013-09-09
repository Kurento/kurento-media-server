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

  GST_TRACE ("addHandlerAddress %d, %s, %d", handlerId, address.c_str(), port);

  try {
    mediaHandlerMutex.lock ();
    mh = mediaHandlerMap.getValue (handlerId);

    if (mh == NULL) {
      mh = std::shared_ptr<MediaHandler> (new MediaHandler (handlerId) );
      mediaHandlerMap.put (handlerId, mh);
    }

    mediaHandlerMutex.unlock ();

    mha = std::shared_ptr<MediaHandlerAddress> (new MediaHandlerAddress (address, port) );
    mh->addAddress (mha);
  } catch (...) {
    GST_TRACE ("addHandlerAddress %d, %s, %d throws MediaServerException", handlerId, address.c_str(), port);
    throw MediaServerException();
  }

  GST_TRACE ("addHandlerAddress %d, %s, %d done", handlerId, address.c_str(), port);
}

/* MediaObject */

void
MediaServerServiceHandler::release (const MediaObjectId &mediaObject) throw (MediaObjectNotFoundException, MediaServerException)
{
  GST_TRACE ("release %" G_GUINT64_FORMAT, mediaObject.id);

  try {
    mediaSet.remove (mediaObject);
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("release %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaObject.id);
    throw e;
  } catch (...) {
    GST_TRACE ("release %" G_GUINT64_FORMAT " throws MediaServerException", mediaObject.id);
    throw MediaServerException();
  }

  GST_TRACE ("release %" G_GUINT64_FORMAT " done", mediaObject.id);
}

void
MediaServerServiceHandler::getParent (MediaObjectId &_return, const MediaObjectId &mediaObject)
throw (MediaObjectNotFoundException, NoParentException, MediaServerException)
{
  std::shared_ptr<MediaObjectImpl> mo;
  std::shared_ptr<MediaObjectId> parent;

  GST_TRACE ("getParent %" G_GUINT64_FORMAT, mediaObject.id);

  try {
    mo = mediaSet.getMediaObject<MediaObjectImpl> (mediaObject);
    parent = mo->getParent ();
    _return = *parent;
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("getParent %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaObject.id);
    throw e;
  } catch (const NoParentException &e) {
    GST_TRACE ("getParent %" G_GUINT64_FORMAT " throws NoParentException", mediaObject.id);
    throw e;
  } catch (...) {
    throw MediaServerException();
  }

  GST_TRACE ("getParent %" G_GUINT64_FORMAT " done", mediaObject.id);
}

/* MediaPipeline */

void
MediaServerServiceHandler::createMediaPipeline (MediaObjectId &_return, const int32_t handlerId)
throw (MediaObjectNotFoundException, HandlerNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaHandler> mh;
  std::shared_ptr<MediaPipeline> mediaPipeline;

  GST_TRACE ("createMediaPipeline");

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
    GST_TRACE ("createMediaPipeline throws MediaObjectNotFoundException");
    throw e;
  } catch (const HandlerNotFoundException &e) {
    GST_TRACE ("createMediaPipeline throws HandlerNotFoundException");
    throw e;
  } catch (...) {
    GST_TRACE ("createMediaPipeline throws MediaServerException");
    throw MediaServerException();
  }

  GST_TRACE ("createMediaPipeline id: %ld done", mediaPipeline->id );
}

void
MediaServerServiceHandler::createSdpEndPoint (MediaObjectId &_return, const MediaObjectId &mediaPipeline,
    const SdpEndPointType::type type) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<SdpEndPoint> sdpEp;

  GST_TRACE ("createSdpEndPoint pipeline: %" G_GUINT64_FORMAT ", type: %s", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type) );

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    sdpEp = mm->createSdpEndPoint (type);
    mediaSet.put (sdpEp);

    _return = *sdpEp;
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("createSdpEndPoint pipeline: %" G_GUINT64_FORMAT ", type: %s throws MediaObjectNotFoundException", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type) );
    throw e;
  } catch (...) {
    GST_TRACE ("createSdpEndPoint pipeline: %" G_GUINT64_FORMAT ", type: %s throws MediaServerException", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type) );
    throw MediaServerException();
  }

  GST_TRACE ("createSdpEndPoint pipeline: %" G_GUINT64_FORMAT ", type: %s done", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type) );
}

void
MediaServerServiceHandler::createSdpEndPointWithFixedSdp (MediaObjectId &_return, const MediaObjectId &mediaPipeline,
    const SdpEndPointType::type type, const std::string &sdp)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<SdpEndPoint> sdpEp;

  GST_TRACE ("createSdpEndPointWithFixedSdp pipeline: %" G_GUINT64_FORMAT ", type: %s, sdp: %s", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type), sdp.c_str() );

  try {
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    sdpEp = mm->createSdpEndPoint (type, sdp);
    mediaSet.put (sdpEp);

    _return = *sdpEp;
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("createSdpEndPointWithFixedSdp pipeline: %" G_GUINT64_FORMAT ", type: %s, sdp: %s throws MediaObjectNotFoundException", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type), sdp.c_str() );
    throw e;
  } catch (...) {
    GST_TRACE ("createSdpEndPointWithFixedSdp pipeline: %" G_GUINT64_FORMAT ", type: %s, sdp: %s throws MediaServerException", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type), sdp.c_str() );
    throw MediaServerException();
  }

  GST_TRACE ("createSdpEndPointWithFixedSdp pipeline: %" G_GUINT64_FORMAT ", type: %s, sdp: %s done", mediaPipeline.id, _SdpEndPointType_VALUES_TO_NAMES.at (type), sdp.c_str() );
}

void
MediaServerServiceHandler::createUriEndPoint (MediaObjectId &_return, const MediaObjectId &mediaPipeline, const UriEndPointType::type type,
    const std::string &uri) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPipeline> mm;
  std::shared_ptr<UriEndPoint> uriEp;

  try {
    GST_TRACE ("createUriEndPoint pipeline: %" G_GUINT64_FORMAT " type %s", mediaPipeline.id, _UriEndPointType_VALUES_TO_NAMES.at (type) );
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    uriEp = mm->createUriEndPoint (type, uri);
    mediaSet.put (uriEp);

    _return = *uriEp;
    GST_TRACE ("createUriEndPoint pipeline: %" G_GUINT64_FORMAT " type %s done", mediaPipeline.id, _UriEndPointType_VALUES_TO_NAMES.at (type) );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("createUriEndPoint pipeline: %" G_GUINT64_FORMAT " type %s throws MediaObjectNotFoundException", mediaPipeline.id, _UriEndPointType_VALUES_TO_NAMES.at (type) );
    throw e;
  } catch (...) {
    GST_TRACE ("createUriEndPoint pipeline: %" G_GUINT64_FORMAT " type %s throws MediaServerException", mediaPipeline.id, _UriEndPointType_VALUES_TO_NAMES.at (type) );
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

  url = kms_http_ep_server_register_end_point (httpepserver, httpEp->element);

  if (url == NULL)
    return FALSE;

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
    GST_TRACE ("createHttpEndPoint pipeline: %" G_GUINT64_FORMAT, mediaPipeline.id);
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    httpEp = mm->createHttpEndPoint ();
    mediaSet.put (httpEp);

    operate_in_main_loop_context (register_http_end_point, &*httpEp, NULL);

    _return = *httpEp;
    GST_TRACE ("createHttpEndPoint pipeline: %" G_GUINT64_FORMAT " done", mediaPipeline.id);
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("createHttpEndPoint pipeline: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaPipeline.id);
    throw e;
  } catch (...) {
    GST_TRACE ("createHttpEndPoint pipeline: %" G_GUINT64_FORMAT " throws MediaServerException", mediaPipeline.id);
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
    GST_TRACE ("createMixer pipeline: %" G_GUINT64_FORMAT " type %s", mediaPipeline.id, _MixerType_VALUES_TO_NAMES.at (type) );
    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    mixer = mm->createMixer (type);
    mediaSet.put (mixer);

    _return = *mixer;
    GST_TRACE ("createMixer pipeline: %" G_GUINT64_FORMAT " type %s done", mediaPipeline.id, _MixerType_VALUES_TO_NAMES.at (type) );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("createMixer pipeline: %" G_GUINT64_FORMAT " type %s throws MediaObjectNotFoundException", mediaPipeline.id, _MixerType_VALUES_TO_NAMES.at (type) );
    throw e;
  } catch (...) {
    GST_TRACE ("createMixer pipeline: %" G_GUINT64_FORMAT " type %s throws MediaServerException", mediaPipeline.id, _MixerType_VALUES_TO_NAMES.at (type) );
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
    GST_TRACE ("createFilter pipeline: %" G_GUINT64_FORMAT " type %s", mediaPipeline.id, _FilterType_VALUES_TO_NAMES.at (type) );

    mm = mediaSet.getMediaObject<MediaPipeline> (mediaPipeline);
    filter = mm->createFilter (type);
    mediaSet.put (filter);

    _return = *filter;
    GST_TRACE ("createFilter pipeline: %" G_GUINT64_FORMAT " type %s done", mediaPipeline.id, _FilterType_VALUES_TO_NAMES.at (type) );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("createFilter pipeline: %" G_GUINT64_FORMAT " type %s throws MediaObjectNotFoundException", mediaPipeline.id, _FilterType_VALUES_TO_NAMES.at (type) );
    throw e;
  } catch (...) {
    GST_TRACE ("createFilter pipeline: %" G_GUINT64_FORMAT " type %s throws MediaServerException", mediaPipeline.id, _FilterType_VALUES_TO_NAMES.at (type) );
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
    GST_TRACE ("getMediaSrcs element: %" G_GUINT64_FORMAT, mediaElement.id);
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSrcs = me->getMediaSrcs();

    for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSrcs;
    GST_TRACE ("getMediaSrcs element: %" G_GUINT64_FORMAT " done", mediaElement.id);
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSrcs;
    GST_TRACE ("getMediaSrcs element: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaElement.id);
    throw e;
  } catch (...) {
    delete mediaSrcs;
    GST_TRACE ("getMediaSrcs element: %" G_GUINT64_FORMAT " throws MediaServerException", mediaElement.id);
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
    GST_TRACE ("getMediaSinks element: %" G_GUINT64_FORMAT, mediaElement.id);
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSinks = me->getMediaSinks();

    for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSinks;
    GST_TRACE ("getMediaSinks element: %" G_GUINT64_FORMAT " done", mediaElement.id);
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSinks;
    GST_TRACE ("getMediaSinks element: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaElement.id);
    throw e;
  } catch (...) {
    delete mediaSinks;
    GST_TRACE ("getMediaSinks element: %" G_GUINT64_FORMAT " throws MediaServerException", mediaElement.id);
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
    GST_TRACE ("getMediaSrcByType element: %" G_GUINT64_FORMAT " mediaType: %s", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSrcs = me->getMediaSrcsByMediaType (mediaType);

    for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSrcs;
    GST_TRACE ("getMediaSrcsByType element: %" G_GUINT64_FORMAT " mediaType: %s done", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSrcs;
    GST_TRACE ("getMediaSrcsByType element: %" G_GUINT64_FORMAT " mediaType: %s throws MediaObjectNotFoundException", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
    throw e;
  } catch (...) {
    delete mediaSrcs;
    GST_TRACE ("getMediaSrcsByType element: %" G_GUINT64_FORMAT " mediaType: %s throws MediaServerException", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
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
    GST_TRACE ("getMediaSinksByType element: %" G_GUINT64_FORMAT " mediaType: %s", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
    me = mediaSet.getMediaObject<MediaElement> (mediaElement);
    mediaSinks = me->getMediaSinksByMediaType (mediaType);

    for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSinks;
    GST_TRACE ("getMediaSinksByType element: %" G_GUINT64_FORMAT " mediaType: %s done", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSinks;
    GST_TRACE ("getMediaSinksByType element: %" G_GUINT64_FORMAT " mediaType: %s throws MediaObjectNotFoundException", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
    throw e;
  } catch (...) {
    delete mediaSinks;
    GST_TRACE ("getMediaSinksByType element: %" G_GUINT64_FORMAT " mediaType: %s throws MediaServerException", mediaElement.id, _MediaType_VALUES_TO_NAMES.at (mediaType) );
    throw MediaServerException();
  }
}

/* MediaPad */

MediaType::type
MediaServerServiceHandler::getMediaType (const MediaObjectId &mediaPad) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPad> pad;

  try {
    GST_TRACE ("getMediaType pad: %" G_GUINT64_FORMAT, mediaPad.id);
    pad = mediaSet.getMediaObject<MediaPad> (mediaPad);
    GST_TRACE ("getMediaType pad: %" G_GUINT64_FORMAT " done", mediaPad.id);
    return pad->getMediaType ();
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("getMediaType pad: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaPad.id);
    throw e;
  } catch (...) {
    GST_TRACE ("getMediaType pad: %" G_GUINT64_FORMAT " throws MediaServerException", mediaPad.id);
    throw MediaServerException();
  }
}

/* MediaSrc */

void
MediaServerServiceHandler::connect (const MediaObjectId &mediaSrc, const MediaObjectId &mediaSink)
throw (MediaObjectNotFoundException, ConnectionException, MediaServerException)
{
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  try {
    GST_TRACE ("connect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT, mediaSrc.id, mediaSink.id);
    src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
    sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
    src->connect (sink);
    GST_TRACE ("connect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT " done", mediaSrc.id, mediaSink.id);
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("connect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaSrc.id, mediaSink.id);
    throw e;
  } catch (const ConnectionException &e) {
    GST_TRACE ("connect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT " throws ConnectionException", mediaSrc.id, mediaSink.id);
    throw e;
  } catch (...) {
    GST_TRACE ("connect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT " throws MediaServerException", mediaSrc.id, mediaSink.id);
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
    GST_TRACE ("disconnect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT, mediaSrc.id, mediaSink.id);
    src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
    sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
    src->disconnect (sink);
    GST_TRACE ("disconnect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT " done", mediaSrc.id, mediaSink.id);
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("disconnect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaSrc.id, mediaSink.id);
    throw e;
  } catch (...) {
    GST_TRACE ("disconnect src: %" G_GUINT64_FORMAT " sink: %" G_GUINT64_FORMAT " throws MediaServerException", mediaSrc.id, mediaSink.id);
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
    GST_TRACE ("getConnectedSinks src: %" G_GUINT64_FORMAT, mediaSrc.id);
    src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
    mediaSinks = src->getConnectedSinks();

    for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
      mediaSet.put (*it);
      _return.push_back (**it);
    }

    delete mediaSinks;
    GST_TRACE ("getConnectedSinks src: %" G_GUINT64_FORMAT " done", mediaSrc.id);
  } catch (const MediaObjectNotFoundException &e) {
    delete mediaSinks;
    GST_TRACE ("getConnectedSinks src: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaSrc.id);
    throw e;
  } catch (...) {
    delete mediaSinks;
    GST_TRACE ("getConnectedSinks src: %" G_GUINT64_FORMAT " throws MediaServerException", mediaSrc.id);
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
    GST_TRACE ("getConnectedSrc sink: %" G_GUINT64_FORMAT, mediaSink.id);
    sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
    _return = * (sink->getConnectedSrc() );
    GST_TRACE ("getConnectedSrc sink: %" G_GUINT64_FORMAT " done", mediaSink.id);
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("getConnectedSrc sink: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mediaSink.id);
    throw e;
  } catch (...) {
    GST_TRACE ("getConnectedSrc sink: %" G_GUINT64_FORMAT " throws MediaServerException", mediaSink.id);
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

  GST_TRACE ("createMixerEndPoint mixer: %" G_GUINT64_FORMAT, mixer.id);

  try {
    m = mediaSet.getMediaObject<Mixer> (mixer);
    mixerEndPoint = m->createMixerEndPoint();
    mediaSet.put (mixerEndPoint);

    _return = *mixerEndPoint;
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("createMixerEndPoint mixer: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", mixer.id);
    throw e;
  } catch (...) {
    GST_TRACE ("createMixerEndPoint mixer: %" G_GUINT64_FORMAT " throws MediaServerException", mixer.id);
    throw MediaServerException();
  }

  GST_TRACE ("createMixerEndPoint mixer: %" G_GUINT64_FORMAT " done", mixer.id);
}

/* HttpEndPoint */

void
MediaServerServiceHandler::getUrl (std::string &_return, const MediaObjectId &httpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<HttpEndPoint> httpEp;

  GST_TRACE ("getUrl httpEndPoint: %" G_GUINT64_FORMAT, httpEndPoint.id);

  try {
    httpEp = mediaSet.getMediaObject<HttpEndPoint> (httpEndPoint);
    _return.assign (httpEp->getUrl () );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("getUrl httpEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", httpEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("getUrl httpEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", httpEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("getUrl httpEndPoint: %" G_GUINT64_FORMAT " done", httpEndPoint.id);
}

/* UriEndPoint */

void
MediaServerServiceHandler::getUri (std::string &_return, const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  GST_TRACE ("getUri uriEndPoint: %" G_GUINT64_FORMAT, uriEndPoint.id);

  try {
    uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
    _return.assign (uep->getUri() );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("getUri uriEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", uriEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("getUri uriEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", uriEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("getUri uriEndPoint: %" G_GUINT64_FORMAT " done", uriEndPoint.id);
}

void
MediaServerServiceHandler::start (const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  GST_TRACE ("start uriEndPoint: %" G_GUINT64_FORMAT, uriEndPoint.id);

  try {
    uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
    uep->start();
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("start uriEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", uriEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("start uriEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", uriEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("start uriEndPoint: %" G_GUINT64_FORMAT " done", uriEndPoint.id);
}

void
MediaServerServiceHandler::pause (const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  GST_TRACE ("pause uriEndPoint: %" G_GUINT64_FORMAT, uriEndPoint.id);

  try {
    uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
    uep->pause();
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("pause uriEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", uriEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("pause uriEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", uriEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("pause uriEndPoint: %" G_GUINT64_FORMAT " done", uriEndPoint.id);
}

void
MediaServerServiceHandler::stop (const MediaObjectId &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  GST_TRACE ("stop uriEndPoint: %" G_GUINT64_FORMAT, uriEndPoint.id);

  try {
    uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
    uep->stop();
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("stop uriEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", uriEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("stop uriEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", uriEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("stop uriEndPoint: %" G_GUINT64_FORMAT " done", uriEndPoint.id);
}

/* SdpEndPoint */

void
MediaServerServiceHandler::generateOffer (std::string &_return, const MediaObjectId &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  GST_TRACE ("generateOffer sdpEndPoint: %" G_GUINT64_FORMAT, sdpEndPoint.id);

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->generateOffer () );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("generateOffer sdpEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", sdpEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("generateOffer sdpEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", sdpEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("generateOffer sdpEndPoint: %" G_GUINT64_FORMAT " done", sdpEndPoint.id);
}

void
MediaServerServiceHandler::processAnswer (std::string &_return, const MediaObjectId &sdpEndPoint, const std::string &answer)
throw (MediaObjectNotFoundException, NegotiationException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  GST_TRACE ("processAnswer sdpEndPoint: %" G_GUINT64_FORMAT ", answer: %s", sdpEndPoint.id, answer.c_str() );

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->processAnswer (answer) );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("processAnswer sdpEndPoint: %" G_GUINT64_FORMAT ", answer: %s throws MediaObjectNotFoundException", sdpEndPoint.id, answer.c_str() );
    throw e;
  } catch (const NegotiationException &e) {
    GST_TRACE ("processAnswer sdpEndPoint: %" G_GUINT64_FORMAT ", answer: %s throws NegotiationException", sdpEndPoint.id, answer.c_str() );
    throw e;
  } catch (...) {
    GST_TRACE ("processAnswer sdpEndPoint: %" G_GUINT64_FORMAT ", answer: %s throws MediaServerException", sdpEndPoint.id, answer.c_str() );
    throw MediaServerException();
  }

  GST_TRACE ("processAnswer sdpEndPoint: %" G_GUINT64_FORMAT ", answer: %s done", sdpEndPoint.id, answer.c_str() );
}

void
MediaServerServiceHandler::processOffer (std::string &_return, const MediaObjectId &sdpEndPoint, const std::string &offer)
throw (MediaObjectNotFoundException, NegotiationException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  GST_TRACE ("processOffer sdpEndPoint: %" G_GUINT64_FORMAT ", offer: %s", sdpEndPoint.id, offer.c_str() );

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->processOffer (offer) );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("processOffer sdpEndPoint: %" G_GUINT64_FORMAT ", offer: %s throws MediaObjectNotFoundException", sdpEndPoint.id, offer.c_str() );
    throw e;
  } catch (const NegotiationException &e) {
    GST_TRACE ("processOffer sdpEndPoint: %" G_GUINT64_FORMAT ", offer: %s throws NegotiationException", sdpEndPoint.id, offer.c_str() );
    throw e;
  } catch (...) {
    GST_TRACE ("processOffer sdpEndPoint: %" G_GUINT64_FORMAT ", offer: %s throws MediaServerException", sdpEndPoint.id, offer.c_str() );
    throw MediaServerException();
  }

  GST_TRACE ("processOffer sdpEndPoint: %" G_GUINT64_FORMAT ", offer: %s done", sdpEndPoint.id, offer.c_str() );
}

void
MediaServerServiceHandler::getLocalSessionDescription (std::string &_return, const MediaObjectId &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  GST_TRACE ("getLocalSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT, sdpEndPoint.id);

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->getLocalSessionDescription () );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("getLocalSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", sdpEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("getLocalSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", sdpEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("getLocalSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT " done", sdpEndPoint.id);
}

void
MediaServerServiceHandler::getRemoteSessionDescription (std::string &_return, const MediaObjectId &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  GST_TRACE ("getRemoteSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT, sdpEndPoint.id);

  try {
    s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
    _return.assign (s->getRemoteSessionDescription () );
  } catch (const MediaObjectNotFoundException &e) {
    GST_TRACE ("getRemoteSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT " throws MediaObjectNotFoundException", sdpEndPoint.id);
    throw e;
  } catch (...) {
    GST_TRACE ("getRemoteSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT " throws MediaServerException", sdpEndPoint.id);
    throw MediaServerException();
  }

  GST_TRACE ("getRemoteSessionDescription sdpEndPoint: %" G_GUINT64_FORMAT " done", sdpEndPoint.id);
}

MediaServerServiceHandler::StaticConstructor MediaServerServiceHandler::staticConstructor;

MediaServerServiceHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
