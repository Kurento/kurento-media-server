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
#include <gst/gst.h>

#include "types/MediaFactory.hpp"
#include "types/MediaPlayer.hpp"
#include "types/MediaRecorder.hpp"
#include "types/Stream.hpp"
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

void
MediaServerServiceHandler::ping (const MediaObject &resource,
    const int32_t timeout)
{
  GST_INFO ("ping");
}

void
MediaServerServiceHandler::release (const MediaObject &mediaObject)
{
  mediaSet.remove (mediaObject);
}

void
MediaServerServiceHandler::createMediaFactory (MediaObject &_return)
{
  std::shared_ptr<MediaFactory> mediaFactory;

  mediaFactory = std::shared_ptr<MediaFactory> (new MediaFactory() );
  GST_DEBUG ("createMediaFactory id: %ld, token: %s", mediaFactory->id, mediaFactory->token.c_str() );
  mediaSet.put (mediaFactory);
  GST_INFO ("%d active media factories", mediaSet.size() );

  _return = *mediaFactory;
}

void
MediaServerServiceHandler::createMediaPlayer (MediaObject &_return,
    const MediaObject &mediaFactory)
{
  std::shared_ptr<MediaFactory> mf;
  std::shared_ptr<MediaPlayer> mediaPlayer;

  mf = mediaSet.getMediaObject<MediaFactory> (mediaFactory);
  mediaPlayer =  mf->createMediaPlayer();
  mediaSet.put (mediaPlayer);

  _return = *mediaPlayer;
}

void
MediaServerServiceHandler::createMediaRecorder (MediaObject &_return,
    const MediaObject &mediaFactory)
{
  std::shared_ptr<MediaFactory> mf;
  std::shared_ptr<MediaRecorder> mediaRecorder;

  mf = mediaSet.getMediaObject<MediaFactory> (mediaFactory);
  mediaRecorder = mf->createMediaRecorder();
  mediaSet.put (mediaRecorder);

  _return = *mediaRecorder;
}

void
MediaServerServiceHandler::createStream (MediaObject &_return,
    const MediaObject &mediaFactory)
{
  std::shared_ptr<MediaFactory> mf;
  std::shared_ptr<Stream> stream;

  mf = mediaSet.getMediaObject<MediaFactory> (mediaFactory);
  stream = mf->createStream();
  mediaSet.put (stream);

  _return = *stream;
}

void
MediaServerServiceHandler::createMixer (MediaObject &_return,
    const MediaObject &mediaFactory, const int32_t mixerId)
{
  std::shared_ptr<MediaFactory> mf;
  std::shared_ptr<Mixer> mixer;

  mf = mediaSet.getMediaObject<MediaFactory> (mediaFactory);
  mixer = mf->createMixer (mixerId);
  mediaSet.put (mixer);

  _return = *mixer;
}

MediaType::type
MediaServerServiceHandler::getMediaType (const MediaObject &mediaElement)
{
  GST_INFO ("getMediaType");
  //FIXME: use two calls, one for MediaSrc and other for MediaSink
  return MediaType::AUDIO;
}

void
MediaServerServiceHandler::connect (const MediaObject &mediaSrc,
    const MediaObject &mediaSink)
{
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
  sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
  src->connect (*sink);
}

void
MediaServerServiceHandler::disconnect (const MediaObject &mediaSrc,
    const MediaObject &mediaSink)
{
  std::shared_ptr<MediaSrc> src;
  std::shared_ptr<MediaSink> sink;

  src = mediaSet.getMediaObject<MediaSrc> (mediaSrc);
  sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
  src->disconnect (*sink);
}

void
MediaServerServiceHandler::getConnectedSinks (std::vector < MediaObject >
    &_return, const MediaObject &mediaSrc)
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

void
MediaServerServiceHandler::getConnectedSrc (MediaObject &_return,
    const MediaObject &mediaSink)
{
  std::shared_ptr<MediaSink> sink;

  sink = mediaSet.getMediaObject<MediaSink> (mediaSink);
  _return = * (sink->getConnectedSrc() );
}

void
MediaServerServiceHandler::join (const MediaObject &joinableA,
    const MediaObject &joinableB)
{
  std::shared_ptr<Joinable> jA, jB;

  jA = mediaSet.getMediaObject<Joinable> (joinableA);
  jB = mediaSet.getMediaObject<Joinable> (joinableB);
  jA->join (*jB);
}

void
MediaServerServiceHandler::unjoin (const MediaObject &joinableA,
    const MediaObject &joinableB)
{
  std::shared_ptr<Joinable> jA, jB;

  jA = mediaSet.getMediaObject<Joinable> (joinableA);
  jB = mediaSet.getMediaObject<Joinable> (joinableB);
  jA->unjoin (*jB);
}

void
MediaServerServiceHandler::getMediaSrcs (std::vector < MediaObject > &_return,
    const MediaObject &joinable)
{
  std::shared_ptr<Joinable> j;
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs;
  std::vector< std::shared_ptr<MediaSrc> >::iterator it;

  j = mediaSet.getMediaObject<Joinable> (joinable);
  mediaSrcs = j->getMediaSrcs();

  for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSrcs;
}

void
MediaServerServiceHandler::getMediaSinks (std::vector < MediaObject > &_return,
    const MediaObject &joinable)
{
  std::shared_ptr<Joinable> j;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  j = mediaSet.getMediaObject<Joinable> (joinable);
  mediaSinks = j->getMediaSinks();

  for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSinks;
}

void
MediaServerServiceHandler::getMediaSrcsByMediaType (std::vector < MediaObject >
    &_return, const MediaObject &joinable, const MediaType::type mediaType)
{
  std::shared_ptr<Joinable> j;
  std::vector < std::shared_ptr<MediaSrc> > *mediaSrcs;
  std::vector< std::shared_ptr<MediaSrc> >::iterator it;

  j = mediaSet.getMediaObject<Joinable> (joinable);
  mediaSrcs = j->getMediaSrcsByMediaType (mediaType);

  for ( it = mediaSrcs->begin() ; it != mediaSrcs->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSrcs;
}

void
MediaServerServiceHandler::getMediaSinksByMediaType (std::vector < MediaObject >
    &_return, const MediaObject &joinable, const MediaType::type mediaType)
{
  std::shared_ptr<Joinable> j;
  std::vector < std::shared_ptr<MediaSink> > *mediaSinks;
  std::vector< std::shared_ptr<MediaSink> >::iterator it;

  j = mediaSet.getMediaObject<Joinable> (joinable);
  mediaSinks = j->getMediaSinksByMediaType (mediaType);

  for ( it = mediaSinks->begin() ; it != mediaSinks->end(); ++it) {
    mediaSet.put (*it);
    _return.push_back (**it);
  }

  delete mediaSinks;
}

void
MediaServerServiceHandler::play (const MediaObject &mediaPlayer)
{
  std::shared_ptr<MediaPlayer> mp;

  mp = mediaSet.getMediaObject<MediaPlayer> (mediaPlayer);
  mp->play();
}

void
MediaServerServiceHandler::pausePlayer (const MediaObject &mediaPlayer)
{
  std::shared_ptr<MediaPlayer> mp;

  mp = mediaSet.getMediaObject<MediaPlayer> (mediaPlayer);
  mp->pause();
}

void
MediaServerServiceHandler::stopPlayer (const MediaObject &mediaPlayer)
{
  std::shared_ptr<MediaPlayer> mp;

  mp = mediaSet.getMediaObject<MediaPlayer> (mediaPlayer);
  mp->stop();
}

void
MediaServerServiceHandler::record (const MediaObject &mediaRecorder)
{
  std::shared_ptr<MediaRecorder> mr;

  mr = mediaSet.getMediaObject<MediaRecorder> (mediaRecorder);
  mr->record();
}

void
MediaServerServiceHandler::pauseRecorder (const MediaObject &mediaRecorder)
{
  std::shared_ptr<MediaRecorder> mr;

  mr = mediaSet.getMediaObject<MediaRecorder> (mediaRecorder);
  mr->pause();
}

void
MediaServerServiceHandler::stopRecorder (const MediaObject &mediaRecorder)
{
  std::shared_ptr<MediaRecorder> mr;

  mr = mediaSet.getMediaObject<MediaRecorder> (mediaRecorder);
  mr->stop();
}

void
MediaServerServiceHandler::generateOffer (std::string &_return,
    const MediaObject &stream)
{
  std::shared_ptr<Stream> s;

  s = mediaSet.getMediaObject<Stream> (stream);
  s->generateOffer (_return);
}

void
MediaServerServiceHandler::processAnswer (std::string &_return,
    const MediaObject &stream, const std::string &answer)
{
  std::shared_ptr<Stream> s;

  s = mediaSet.getMediaObject<Stream> (stream);
  s->processAnswer (_return, answer);
}

void
MediaServerServiceHandler::processOffer (std::string &_return,
    const MediaObject &stream, const std::string &offer)
{
  std::shared_ptr<Stream> s;

  s = mediaSet.getMediaObject<Stream> (stream);
  s->processOffer (_return, offer);
}

void
MediaServerServiceHandler::getLocalDescriptor (std::string &_return,
    const MediaObject &stream)
{
  std::shared_ptr<Stream> s;

  s = mediaSet.getMediaObject<Stream> (stream);
  s->getLocalDescriptor (_return);
}

void
MediaServerServiceHandler::getRemoteDescriptor (std::string &_return,
    const MediaObject &stream)
{
  std::shared_ptr<Stream> s;

  s = mediaSet.getMediaObject<Stream> (stream);
  s->getRemoteDescriptor (_return);
}

void
MediaServerServiceHandler::getMixerPort (MediaObject &_return,
    const MediaObject &mixer)
{
  std::shared_ptr<Mixer> m;
  std::shared_ptr<MixerPort> mixerPort;

  m = mediaSet.getMediaObject<Mixer> (mixer);
  mixerPort = m->getMixerPort();
  mediaSet.put (mixerPort);

  _return = *mixerPort;
}

void
MediaServerServiceHandler::getMixer (MediaObject &_return,
    const MediaObject &mixerPort)
{
  std::shared_ptr<MixerPort> mp;

  mp = mediaSet.getMediaObject<MixerPort> (mixerPort);
  _return = * ( mp->getMixer() );
}

MediaServerServiceHandler::StaticConstructor MediaServerServiceHandler::staticConstructor;

MediaServerServiceHandler::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
