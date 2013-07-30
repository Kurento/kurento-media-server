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
#include "common/MediaSet.hpp"

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
  void release (const MediaObject& mediaObject) throw(MediaObjectNotFoundException, MediaServerException);
  void getParent (MediaObject& _return, const MediaObject& mediaObject) throw(NoParentException);

  /* MediaManager */
  void createMediaManager (MediaObject& _return, const int32_t handlerId)
                          throw(MediaObjectNotFoundException, HandlerNotFoundException, MediaServerException);
  void createSdpEndPoint (MediaObject& _return, const MediaObject& mediaManager, const SdpEndPointType::type type)
                          throw(MediaObjectNotFoundException, MediaServerException);
  void createSdpEndPointWithFixedSdp (MediaObject& _return, const MediaObject& mediaManager, const SdpEndPointType::type type,
                                      const std::string& sdp) throw(MediaObjectNotFoundException, MediaServerException);
  void createUriEndPoint (MediaObject& _return, const MediaObject& mediaManager, const UriEndPointType::type type,
                          const std::string& uri) throw(MediaObjectNotFoundException, MediaServerException);
  void createHttpEndPoint (MediaObject& _return, const MediaObject& mediaManager)
                          throw(MediaObjectNotFoundException, MediaServerException);
  void createMixer (MediaObject& _return, const MediaObject& mediaManager, const MixerType::type type)
                   throw(MediaObjectNotFoundException, MediaServerException);
  void createFilter (MediaObject& _return, const MediaObject& mediaManager, const FilterType::type type)
                    throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaElement */
  void sendCommand (CommandResult& _return, const MediaObject& mediaElement, const Command& command)
                    throw(MediaObjectNotFoundException, EncodingException, MediaServerException);
  void getMediaSrcs (std::vector<MediaObject> & _return, const MediaObject& mediaElement) throw(MediaObjectNotFoundException, MediaServerException);
  void getMediaSinks (std::vector<MediaObject> & _return, const MediaObject& mediaElement) throw(MediaObjectNotFoundException, MediaServerException);
  void getMediaSrcsByMediaType (std::vector<MediaObject> & _return, const MediaObject& mediaElement,
                                const MediaType::type mediaType) throw(MediaObjectNotFoundException, MediaServerException);
  void getMediaSinksByMediaType (std::vector<MediaObject> & _return, const MediaObject& mediaElement,
                                 const MediaType::type mediaType) throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaPad */
  MediaType::type getMediaType (const MediaObject& mediaPad) throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaSrc */
  void connect (const MediaObject& mediaSrc, const MediaObject& mediaSink) throw(MediaObjectNotFoundException, ConnectionException, MediaServerException);
  void disconnect (const MediaObject& src, const MediaObject& mediaSink) throw(MediaObjectNotFoundException, MediaServerException);
  void getConnectedSinks (std::vector<MediaObject> & _return, const MediaObject& mediaSrc) throw(MediaObjectNotFoundException, MediaServerException);

  /* MediaSink */
  void getConnectedSrc (MediaObject& _return, const MediaObject& mediaSink) throw(MediaObjectNotFoundException, MediaServerException);

  /* Mixer */
  void createMixerEndPoint (MediaObject& _return, const MediaObject& mixer) throw(MediaObjectNotFoundException, MediaServerException);

  /* HttpEndPoint */
  void getUrl (std::string& _return, const MediaObject& httpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);

  /* UriEndPoint */
  void getUri (std::string& _return, const MediaObject& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void start (const MediaObject& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void pause (const MediaObject& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void stop (const MediaObject& uriEndPoint) throw(MediaObjectNotFoundException, MediaServerException);

  /* SdpEndPoint */
  void generateOffer (std::string& _return, const MediaObject& sdpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void processAnswer (std::string& _return, const MediaObject& sdpEndPoint, const std::string& answer)
                      throw(MediaObjectNotFoundException, NegotiationException, MediaServerException);
  void processOffer (std::string& _return, const MediaObject& sdpEndPoint, const std::string& offer)
                    throw(MediaObjectNotFoundException, NegotiationException, MediaServerException);
  void getLocalSessionDescription (std::string& _return, const MediaObject& sdpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);
  void getRemoteSessionDescription (std::string& _return, const MediaObject& sdpEndPoint) throw(MediaObjectNotFoundException, MediaServerException);

private:
  MediaSet mediaSet;

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
