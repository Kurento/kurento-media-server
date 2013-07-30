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

#include "types/MediaManager.hpp"
#include "types/MediaElement.hpp"
#include "types/SdpEndPoint.hpp"
#include "types/UriEndPoint.hpp"
#include "types/HttpEndPoint.hpp"
#include "types/Mixer.hpp"

#define GST_CAT_DEFAULT media_server_service_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "MediaServerServiceHandler"

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
  // TODO: implement
}

/* MediaObject */

void
MediaServerServiceHandler::release (const MediaObject &mediaObject) throw (MediaObjectNotFoundException, MediaServerException)
{
  mediaSet.remove (mediaObject);
}

void
MediaServerServiceHandler::getParent (MediaObject &_return, const MediaObject &mediaObject) throw (NoParentException)
{
  std::shared_ptr<MediaObjectImpl> mo;
  std::shared_ptr<MediaObject> parent;

  mo = mediaSet.getMediaObject<MediaObjectImpl> (mediaObject);
  parent = mo->getParent ();
  _return = *parent;
}

/* MediaManager */

void
MediaServerServiceHandler::createMediaManager (MediaObject &_return, const int32_t handlerId)
throw (MediaObjectNotFoundException, HandlerNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaManager> mediaManager;

  mediaManager = std::shared_ptr<MediaManager> (new MediaManager() );
  GST_DEBUG ("createMediaManager id: %ld, token: %s", mediaManager->id, mediaManager->token.c_str() );
  mediaSet.put (mediaManager);
  // TODO: register handler

  _return = *mediaManager;
}

void
MediaServerServiceHandler::createSdpEndPoint (MediaObject &_return, const MediaObject &mediaManager,
    const SdpEndPointType::type type) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaManager> mm;
  std::shared_ptr<SdpEndPoint> sdpEp;

  mm = mediaSet.getMediaObject<MediaManager> (mediaManager);
  sdpEp = mm->createSdpEndPoint (type);
  mediaSet.put (sdpEp);

  _return = *sdpEp;
}

void
MediaServerServiceHandler::createSdpEndPointWithFixedSdp (MediaObject &_return, const MediaObject &mediaManager,
    const SdpEndPointType::type type, const std::string &sdp)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaManager> mm;
  std::shared_ptr<SdpEndPoint> sdpEp;

  mm = mediaSet.getMediaObject<MediaManager> (mediaManager);
  sdpEp = mm->createSdpEndPoint (type, sdp);
  mediaSet.put (sdpEp);

  _return = *sdpEp;
}

void
MediaServerServiceHandler::createUriEndPoint (MediaObject &_return, const MediaObject &mediaManager, const UriEndPointType::type type,
    const std::string &uri) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaManager> mm;
  std::shared_ptr<UriEndPoint> uriEp;

  mm = mediaSet.getMediaObject<MediaManager> (mediaManager);
  uriEp = mm->createUriEndPoint (type, uri);
  mediaSet.put (uriEp);

  _return = *uriEp;
}

void
MediaServerServiceHandler::createHttpEndPoint (MediaObject &_return, const MediaObject &mediaManager)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaManager> mm;
  std::shared_ptr<HttpEndPoint> httpEp;

  mm = mediaSet.getMediaObject<MediaManager> (mediaManager);
  httpEp = mm->createHttpEndPoint ();
  mediaSet.put (httpEp);

  _return = *httpEp;
}

void
MediaServerServiceHandler::createMixer (MediaObject &_return, const MediaObject &mediaManager, const MixerType::type type)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaManager> mm;
  std::shared_ptr<Mixer> mixer;

  mm = mediaSet.getMediaObject<MediaManager> (mediaManager);
  mixer = mm->createMixer (type);
  mediaSet.put (mixer);

  _return = *mixer;
}

void
MediaServerServiceHandler::createFilter (MediaObject &_return, const MediaObject &mediaManager, const FilterType::type type)
throw (MediaObjectNotFoundException, MediaServerException)
{
  // TODO: implement
}

/* MediaElement */

void
MediaServerServiceHandler::sendCommand (CommandResult &_return, const MediaObject &mediaElement,
    const Command &command) throw (MediaObjectNotFoundException, EncodingException, MediaServerException)
{
  // TODO: implement
}

void
MediaServerServiceHandler::getMediaSrcs (std::vector<MediaObject> & _return, const MediaObject &mediaElement)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs;
  std::vector< std::shared_ptr<MediaSrc> >::iterator it;

  me = mediaSet.getMediaObject<MediaElement> (mediaElement);
  mediaSrcs = me->getMediaSrcs();

  for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSrcs;
}

void
MediaServerServiceHandler::getMediaSinks (std::vector<MediaObject> & _return, const MediaObject &mediaElement)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  me = mediaSet.getMediaObject<MediaElement> (mediaElement);
  mediaSinks = me->getMediaSinks();

  for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSinks;
}

void
MediaServerServiceHandler::getMediaSrcsByMediaType (std::vector<MediaObject> & _return, const MediaObject &mediaElement,
    const MediaType::type mediaType) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs;
  std::vector< std::shared_ptr<MediaSrc> >::iterator it;

  me = mediaSet.getMediaObject<MediaElement> (mediaElement);
  mediaSrcs = me->getMediaSrcsByMediaType (mediaType);

  for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSrcs;
}

void
MediaServerServiceHandler::getMediaSinksByMediaType (std::vector<MediaObject> & _return, const MediaObject &mediaElement,
    const MediaType::type mediaType) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaElement> me;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  me = mediaSet.getMediaObject<MediaElement> (mediaElement);
  mediaSinks = me->getMediaSinksByMediaType (mediaType);

  for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSinks;
}

/* MediaPad */

MediaType::type
MediaServerServiceHandler::getMediaType (const MediaObject &mediaPad) throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaPad> pad;

  pad = mediaSet.getMediaObject<MediaPad> (mediaPad);
  return pad->getMediaType ();
}

/* MediaSrc */

void
MediaServerServiceHandler::connect (const MediaObject &mediaSrc, const MediaObject &mediaSink)
throw (MediaObjectNotFoundException, ConnectionException, MediaServerException)
{
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
  sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
  src->connect (*sink);
}

void
MediaServerServiceHandler::disconnect (const MediaObject &mediaSrc, const MediaObject &mediaSink)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
  sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
  src->disconnect (*sink);
}

void
MediaServerServiceHandler::getConnectedSinks (std::vector < MediaObject >&_return, const MediaObject &mediaSrc)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaSrc> src;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
  mediaSinks = src->getConnectedSinks();

  for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSinks;
}

/* MediaSink */

void
MediaServerServiceHandler::getConnectedSrc (MediaObject &_return, const MediaObject &mediaSink)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<MediaSink> sink;

  sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
  _return = * (sink->getConnectedSrc() );
}

/* Mixer */

void
MediaServerServiceHandler::createMixerEndPoint (MediaObject &_return, const MediaObject &mixer)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<Mixer> m;
  std::shared_ptr<MixerEndPoint> mixerEndPoint;

  m = mediaSet.getMediaObject<Mixer> (mixer);
  mixerEndPoint = m->createMixerEndPoint();
  mediaSet.put (mixerEndPoint);

  _return = *mixerEndPoint;
}

/* HttpEndPoint */

void
MediaServerServiceHandler::getUrl (std::string &_return, const MediaObject &httpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<HttpEndPoint> httpEp;

  httpEp = mediaSet.getMediaObject<HttpEndPoint> (httpEndPoint);
  _return.assign (httpEp->getUrl () );
}

/* UriEndPoint */

void
MediaServerServiceHandler::getUri (std::string &_return, const MediaObject &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
  _return.assign (uep->getUri() );
}

void
MediaServerServiceHandler::start (const MediaObject &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
  uep->start();
}

void
MediaServerServiceHandler::pause (const MediaObject &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
  uep->pause();
}

void
MediaServerServiceHandler::stop (const MediaObject &uriEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<UriEndPoint> uep;

  uep = mediaSet.getMediaObject<UriEndPoint> (uriEndPoint);
  uep->stop();
}

/* SdpEndPoint */

void
MediaServerServiceHandler::generateOffer (std::string &_return, const MediaObject &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
  _return.assign (s->generateOffer () );
}

void
MediaServerServiceHandler::processAnswer (std::string &_return, const MediaObject &sdpEndPoint, const std::string &answer)
throw (MediaObjectNotFoundException, NegotiationException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
  _return.assign (s->processAnswer (answer) );
}

void
MediaServerServiceHandler::processOffer (std::string &_return, const MediaObject &sdpEndPoint, const std::string &offer)
throw (MediaObjectNotFoundException, NegotiationException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
  _return.assign (s->processOffer (offer) );
}

void
MediaServerServiceHandler::getLocalSessionDescription (std::string &_return, const MediaObject &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
  _return.assign (s->getLocalSessionDescription () );
}

void
MediaServerServiceHandler::getRemoteSessionDescription (std::string &_return, const MediaObject &sdpEndPoint)
throw (MediaObjectNotFoundException, MediaServerException)
{
  std::shared_ptr<SdpEndPoint> s;

  s = mediaSet.getMediaObject<SdpEndPoint> (sdpEndPoint);
  _return.assign (s->getRemoteSessionDescription () );
}

MediaServerServiceHandler::StaticConstructor MediaServerServiceHandler::staticConstructor;

MediaServerServiceHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
