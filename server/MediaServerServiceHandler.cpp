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

#include "types/MediaPlayer.hpp"
#include "types/MediaRecorder.hpp"
#include "types/Stream.hpp"

#define GST_CAT_DEFAULT media_server_service_handler
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "media_server_service_handler"

namespace kurento
{

MediaServerServiceHandler::MediaServerServiceHandler ()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
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
  GST_INFO ("release");
}

void
MediaServerServiceHandler::createMediaFactory (MediaObject &_return)
{
  MediaFactory *mediaFactory;

  mediaFactory = new MediaFactory();
  GST_DEBUG ("createMediaFactory id: %ld, token: %s", mediaFactory->id, mediaFactory->token.c_str() );
  mediaSet.put (mediaFactory);

  GST_INFO ("%d active media factories", mediaSet.size() );

  _return = *mediaFactory;
}

void
MediaServerServiceHandler::createMediaPlayer (MediaObject &_return,
    const MediaObject &mediaFactory)
{
  MediaFactory *mf;
  MediaPlayer *mediaPlayer;

  mf = mediaSet.getMediaFactory (mediaFactory);
  mediaPlayer = mf->createMediaPlayer();
  mediaSet.put (mediaPlayer);

  _return = *mediaPlayer;
}

void
MediaServerServiceHandler::createMediaRecorder (MediaObject &_return,
    const MediaObject &mediaFactory)
{
  MediaFactory *mf;
  MediaRecorder *mediaRecorder;

  mf = mediaSet.getMediaFactory (mediaFactory);
  mediaRecorder = mf->createMediaRecorder();
  mediaSet.put (mediaRecorder);

  _return = *mediaRecorder;
}

void
MediaServerServiceHandler::createStream (MediaObject &_return,
    const MediaObject &mediaFactory)
{
  MediaFactory *mf;
  Stream *stream;

  mf = mediaSet.getMediaFactory (mediaFactory);
  stream = mf->createStream();
  mediaSet.put (stream);

  _return = *stream;
}

void
MediaServerServiceHandler::createMixer (MediaObject &_return,
    const MediaObject &mediaFactory, const int32_t mixerId)
{
  GST_INFO ("createMixer: %d", mixerId);
}

MediaType::type
MediaServerServiceHandler::getMediaType (const MediaObject &mediaElement)
{
  GST_INFO ("getMediaType");
  return MediaType::AUDIO;
}

void
MediaServerServiceHandler::connect (const MediaObject &mediaSrc,
    const MediaObject &mediaSink)
{
  GST_INFO ("connect");
}

void
MediaServerServiceHandler::disconnect (const MediaObject &src,
    const MediaObject &mediaSink)
{
  GST_INFO ("disconnect");
}

void
MediaServerServiceHandler::getConnectedSinks (std::vector < MediaObject >
    &_return, const MediaObject &mediaSrc)
{
  GST_INFO ("getConnectedSinks");
}

void
MediaServerServiceHandler::getConnectedSrc (MediaObject &_return,
    const MediaObject &mediaSink)
{
  GST_INFO ("getConnectedSrc");
}

void
MediaServerServiceHandler::join (const MediaObject &joinableA,
    const MediaObject &joinableB)
{
  GST_INFO ("join");
}

void
MediaServerServiceHandler::unjoin (const MediaObject &joinableA,
    const MediaObject &joinableB)
{
  GST_INFO ("unjoin");
}

void
MediaServerServiceHandler::getMediaSrcs (std::vector < MediaObject > &_return,
    const MediaObject &joinable)
{
  GST_INFO ("getMediaSrcs");
}

void
MediaServerServiceHandler::getMediaSinks (std::vector < MediaObject > &_return,
    const MediaObject &joinable)
{
  GST_INFO ("getMediaSinks");
}

void
MediaServerServiceHandler::getMediaSrcsByMediaType (std::vector < MediaObject >
    &_return, const MediaObject &joinable, const MediaType::type mediaType)
{
  GST_INFO ("getMediaSrcsByMediaType");
}

void
MediaServerServiceHandler::getMediaSinksByMediaType (std::vector < MediaObject >
    &_return, const MediaObject &joinable, const MediaType::type mediaType)
{
  GST_INFO ("getMediaSinksByMediaType");
}

void
MediaServerServiceHandler::play (const MediaObject &mediaPlayer)
{
  MediaPlayer *mp;

  mp = mediaSet.getMediaPlayer (mediaPlayer);
  mp->play();
}

void
MediaServerServiceHandler::pausePlayer (const MediaObject &mediaPlayer)
{
  MediaPlayer *mp;

  mp = mediaSet.getMediaPlayer (mediaPlayer);
  mp->pause();
}

void
MediaServerServiceHandler::stopPlayer (const MediaObject &mediaPlayer)
{
  MediaPlayer *mp;

  mp = mediaSet.getMediaPlayer (mediaPlayer);
  mp->stop();
}

void
MediaServerServiceHandler::record (const MediaObject &mediaRecorder)
{
  MediaRecorder *mr;

  mr = mediaSet.getMediaRecorder (mediaRecorder);
  mr->record();
}

void
MediaServerServiceHandler::pauseRecorder (const MediaObject &mediaRecorder)
{
  MediaRecorder *mr;

  mr = mediaSet.getMediaRecorder (mediaRecorder);
  mr->pause();
}

void
MediaServerServiceHandler::stopRecorder (const MediaObject &mediaRecorder)
{
  MediaRecorder *mr;

  mr = mediaSet.getMediaRecorder (mediaRecorder);
  mr->stop();
}

void
MediaServerServiceHandler::generateOffer (std::string &_return,
    const MediaObject &stream)
{
  GST_INFO ("generateOffer");
  _return.assign ("SessionDescriptor test from generateOffer");
}

void
MediaServerServiceHandler::processAnswer (std::string &_return,
    const MediaObject &stream, const std::string &answer)
{
  GST_INFO ("processAnswer: %s", answer.c_str () );
  _return.assign ("SessionDescriptor test from processAnswer");
}

void
MediaServerServiceHandler::processOffer (std::string &_return,
    const MediaObject &stream, const std::string &offer)
{
  GST_INFO ("processOffer: %s", offer.c_str () );
  _return.assign ("SessionDescriptor test from processOffer");
}

void
MediaServerServiceHandler::getLocalDescriptor (std::string &_return,
    const MediaObject &stream)
{
  GST_INFO ("getLocalDescriptor");
}

void
MediaServerServiceHandler::getRemoteDescriptor (std::string &_return,
    const MediaObject &stream)
{
  GST_INFO ("getRemoteDescriptor");
}

void
MediaServerServiceHandler::getMixerPort (MediaObject &_return,
    const MediaObject &mixer)
{
  GST_INFO ("getMixerPort");
}

void
MediaServerServiceHandler::getMixer (MediaObject &_return,
    const MediaObject &mixerPort)
{
  GST_INFO ("getMixer");
}

} // kurento
