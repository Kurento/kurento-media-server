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

  /* MediaObject */
  void keepAlive(const MediaObjectRef& mediaObjectRef) throw (MediaServerException);
  void release(const MediaObjectRef& mediaObjectRef) throw (MediaServerException);
  void subscribe(std::string& _return, const MediaObjectRef& mediaObjectRef,
		 const std::string& eventType, const std::string& handlerAddress,
		 const int32_t handlerPort)throw (MediaServerException);
  void unsubscribe(const MediaObjectRef& mediaObjectRef, const std::string& callbackToken)
		   throw (MediaServerException);
  void sendCommand(CommandResult& _return, const MediaObjectRef& mediaObjectRef, const Command& command)
		   throw (MediaServerException);
  void getParent(MediaObjectRef& _return, const MediaObjectRef& mediaObjectRef)
		 throw (MediaServerException);
  void getMediaPipeline(MediaObjectRef& _return, const MediaObjectRef& mediaObjectRef)
			throw (MediaServerException);

  /* MediaPileline */
  void createMediaPipeline(MediaObjectRef& _return) throw (MediaServerException);
  void createMediaPipelineWithParams(MediaObjectRef& _return, const Params& params)
				     throw (MediaServerException);
  void createMediaElement(MediaObjectRef& _return, const MediaObjectRef& mediaPipeline,
			  const std::string& elementType) throw (MediaServerException);
  void createMediaElementWithParams(MediaObjectRef& _return, const MediaObjectRef& mediaPipeline,
				    const std::string& elementType, const Params& params)
				    throw (MediaServerException);
  void createMediaMixer(MediaObjectRef& _return, const MediaObjectRef& mediaPipeline,
			const std::string& mixerType) throw (MediaServerException);
  void createMediaMixerWithParams(MediaObjectRef& _return, const MediaObjectRef& mediaPipeline,
				  const std::string& mixerType, const Params& params)
				  throw (MediaServerException);

  /* MediaElement */
  void getMediaSrcs(std::vector<MediaObjectRef> & _return, const MediaObjectRef& mediaElement)
		    throw (MediaServerException);
  void getMediaSinks(std::vector<MediaObjectRef> & _return, const MediaObjectRef& mediaElement)
		     throw (MediaServerException);
  void getMediaSrcsByMediaType(std::vector<MediaObjectRef> & _return, const MediaObjectRef& mediaElement,
			       const MediaType::type mediaType) throw (MediaServerException);
  void getMediaSinksByMediaType(std::vector<MediaObjectRef> & _return, const MediaObjectRef& mediaElement,
				const MediaType::type mediaType) throw (MediaServerException);
  void getMediaSrcsByFullDescription(std::vector<MediaObjectRef> & _return, const MediaObjectRef& mediaElement,
				     const MediaType::type mediaType, const std::string& description)
				     throw (MediaServerException);
  void getMediaSinksByFullDescription(std::vector<MediaObjectRef> & _return, const MediaObjectRef& mediaElement,
				      const MediaType::type mediaType, const std::string& description)
				      throw (MediaServerException);

  /* MediaPad */
  void getMediaElement(MediaObjectRef& _return, const MediaObjectRef& mediaPadRef) throw (MediaServerException);

  /* MediaSrc */
  void connect(const MediaObjectRef& mediaSrc, const MediaObjectRef& mediaSink) throw (MediaServerException);
  void disconnect(const MediaObjectRef& mediaSrc, const MediaObjectRef& mediaSink) throw (MediaServerException);
  void getConnectedSinks(std::vector<MediaObjectRef> & _return, const MediaObjectRef& mediaSrc) throw (MediaServerException);

  /* MediaSink */
  void getConnectedSrc(MediaObjectRef& _return, const MediaObjectRef& mediaSinkRef) throw (MediaServerException);

  /* Mixer */
  void createMixerEndPoint(MediaObjectRef& _return, const MediaObjectRef& mixer) throw (MediaServerException);
  void createMixerEndPointWithParams(MediaObjectRef& _return, const MediaObjectRef& mixer, const Params& params)
				     throw (MediaServerException);

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
