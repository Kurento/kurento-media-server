/*
 * MediaServerServiceHandler.hpp - Kurento Media Server
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

#ifndef __MEDIA_SERVER_SERVICE_HANDLER_HPP__
#define __MEDIA_SERVER_SERVICE_HANDLER_HPP__

#include "MediaServerService.h"
#include "types/MediaHandler.hpp"
#include "common/MediaSet.hpp"
#include "common/ConcurrentMap.hpp"

namespace kurento
{

class MediaServerServiceHandler: public MediaServerServiceIf
{
public:
  MediaServerServiceHandler ();
  ~MediaServerServiceHandler ();

  int32_t getVersion ();

  void addHandlerAddress (const int32_t handlerId, const std::string& address, const int32_t port) throw(MediaServerException);

  /* MediaObject */
  void release (const MediaObjectId& mediaObject) throw(MediaObjectNotFoundException, MediaServerException);
  void getParent (MediaObjectId& _return, const MediaObjectId& mediaObject) throw(NoParentException);

  /* MediaManager */
  void createMediaManager (MediaObjectId& _return, const int32_t handlerId)
                          throw(MediaObjectNotFoundException, HandlerNotFoundException, MediaServerException);
  void createSdpEndPoint (MediaObjectId& _return, const MediaObjectId& mediaManager, const SdpEndPointType::type type)
                          throw(MediaObjectNotFoundException, MediaServerException);
  void createSdpEndPointWithFixedSdp (MediaObjectId& _return, const MediaObjectId& mediaManager, const SdpEndPointType::type type,
                                      const std::string& sdp) throw(MediaObjectNotFoundException, MediaServerException);
  void createUriEndPoint (MediaObjectId& _return, const MediaObjectId& mediaManager, const UriEndPointType::type type,
                          const std::string& uri) throw(MediaObjectNotFoundException, MediaServerException);
  void createHttpEndPoint (MediaObjectId& _return, const MediaObjectId& mediaManager)
                          throw(MediaObjectNotFoundException, MediaServerException);
  void createMixer (MediaObjectId& _return, const MediaObjectId& mediaManager, const MixerType::type type)
                   throw(MediaObjectNotFoundException, MediaServerException);
  void createFilter (MediaObjectId& _return, const MediaObjectId& mediaManager, const FilterType::type type)
                    throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaElement */
  void sendCommand (CommandResult& _return, const MediaObjectId& mediaElement, const Command& command)
                    throw(MediaObjectNotFoundException, EncodingException, MediaServerException);
  void getMediaSrcs (std::vector<MediaObjectId> & _return, const MediaObjectId& mediaElement) throw(MediaObjectNotFoundException, MediaServerException);
  void getMediaSinks (std::vector<MediaObjectId> & _return, const MediaObjectId& mediaElement) throw(MediaObjectNotFoundException, MediaServerException);
  void getMediaSrcsByMediaType (std::vector<MediaObjectId> & _return, const MediaObjectId& mediaElement,
                                const MediaType::type mediaType) throw(MediaObjectNotFoundException, MediaServerException);
  void getMediaSinksByMediaType (std::vector<MediaObjectId> & _return, const MediaObjectId& mediaElement,
                                 const MediaType::type mediaType) throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaPad */
  MediaType::type getMediaType (const MediaObjectId& mediaPad) throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaSrc */
  void connect (const MediaObjectId& mediaSrc, const MediaObjectId& mediaSink) throw(MediaObjectNotFoundException, ConnectionException, MediaServerException);
  void disconnect (const MediaObjectId& src, const MediaObjectId& mediaSink) throw(MediaObjectNotFoundException, MediaServerException);
  void getConnectedSinks (std::vector<MediaObjectId> & _return, const MediaObjectId& mediaSrc) throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaSink */
  void getConnectedSrc (MediaObjectId& _return, const MediaObjectId& mediaSink) throw(MediaObjectNotFoundException, MediaServerException);

  /* Mixer */
  void createMixerEndPoint (MediaObjectId& _return, const MediaObjectId& mixer) throw(MediaObjectNotFoundException, MediaServerException);

  /* HttpEndPoint */
  void getUrl (std::string& _return, const MediaObjectId& httpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);

  /* UriEndPoint */
  void getUri (std::string& _return, const MediaObjectId& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void start (const MediaObjectId& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void pause (const MediaObjectId& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void stop (const MediaObjectId& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);

  /* SdpEndPoint */
  void generateOffer (std::string& _return, const MediaObjectId& sdpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void processAnswer (std::string& _return, const MediaObjectId& sdpEndPoint, const std::string& answer)
                      throw(MediaObjectNotFoundException, NegotiationException, MediaServerException);
  void processOffer (std::string& _return, const MediaObjectId& sdpEndPoint, const std::string& offer)
                    throw(MediaObjectNotFoundException, NegotiationException, MediaServerException);
  void getLocalSessionDescription (std::string& _return, const MediaObjectId& sdpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void getRemoteSessionDescription (std::string& _return, const MediaObjectId& sdpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);

private:
  MediaSet mediaSet;
  Glib::Threads::RecMutex mediaHandlerMutex;
  ConcurrentMap<int32_t, std::shared_ptr<MediaHandler> > mediaHandlerMap;

private:
  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __MEDIA_SERVER_SERVICE_HANDLER_HPP__ */
