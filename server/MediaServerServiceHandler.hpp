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

  void addHandlerAddress (const int32_t handlerId, const std::string& address, const int32_t port);

  /* MediaObject */
  void release (const MediaObject& mediaObject);
  void getParent (MediaObject& _return, const MediaObject& mediaObject);

  /* MediaManager */
  void createMediaManager (MediaObject& _return, const int32_t handlerId);
  void createSdpEndPoint (MediaObject& _return, const MediaObject& mediaManager, const SdpEndPointType::type type);
  void createSdpEndPointWithFixedSdp (MediaObject& _return, const MediaObject& mediaManager, const SdpEndPointType::type type, const std::string& sdp);
  void createUriEndpoint (MediaObject& _return, const MediaObject& mediaManager, const UriEndPointType::type type, const std::string& uri);
  void createHttpEndpoint (MediaObject& _return, const MediaObject& mediaManager);
  void createMixer (MediaObject& _return, const MediaObject& mediaManager, const MixerType::type type);
  void createFilter (MediaObject& _return, const MediaObject& mediaManager, const FilterType::type type);

  /* MediaElement */
  void sendCommand (CommandResult& _return, const MediaObject& mediaElement, const Command& command);
  void getMediaSrcs (std::vector<MediaObject> & _return, const MediaObject& mediaElement);
  void getMediaSinks (std::vector<MediaObject> & _return, const MediaObject& mediaElement);
  void getMediaSrcsByMediaType (std::vector<MediaObject> & _return, const MediaObject& mediaElement, const MediaType::type mediaType);
  void getMediaSinksByMediaType (std::vector<MediaObject> & _return, const MediaObject& mediaElement, const MediaType::type mediaType);

  /* MediaPad */
  MediaType::type getMediaType (const MediaObject& mediaPad);

  /* MediaSrc */
  void connect (const MediaObject& mediaSrc, const MediaObject& mediaSink);
  void disconnect (const MediaObject& src, const MediaObject& mediaSink);
  void getConnectedSinks (std::vector<MediaObject> & _return, const MediaObject& mediaSrc);

  /* MediaSink */
  void getConnectedSrc (MediaObject& _return, const MediaObject& mediaSink);

  /* Mixer */
  void createMixerEndPoint (MediaObject& _return, const MediaObject& mixer);

  /* HttpEndPoint */
  void getUrl (std::string& _return, const MediaObject& httpEndPoint);

  /* UriEndPoint */
  void getUri (std::string& _return, const MediaObject& uriEndPoint);
  void start (const MediaObject& uriEndPoint);
  void pause (const MediaObject& uriEndPoint);
  void stop (const MediaObject& uriEndPoint);

  /* SdpEndPoint */
  void generateOffer (std::string& _return, const MediaObject& sdpEndPoint);
  void processAnswer (std::string& _return, const MediaObject& sdpEndPoint, const std::string& answer);
  void processOffer (std::string& _return, const MediaObject& sdpEndPoint, const std::string& offer);
  void getLocalSessionDescription (std::string& _return, const MediaObject& sdpEndPoint);
  void getRemoteSessionDescription (std::string& _return, const MediaObject& sdpEndPoint);

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
