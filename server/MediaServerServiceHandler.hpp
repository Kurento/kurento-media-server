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

#include "KmsMediaServerService.h"
#include "types/MediaHandler.hpp"
#include "common/MediaSet.hpp"
#include "common/ConcurrentMap.hpp"

namespace kurento
{

class MediaServerServiceHandler: public KmsMediaServerServiceIf
{
public:
  MediaServerServiceHandler ();
  ~MediaServerServiceHandler ();

  int32_t getVersion ();

  /* MediaObject */
  void keepAlive(const KmsMediaObjectRef& mediaObjectRef) throw (KmsMediaServerException);
  void release(const KmsMediaObjectRef& mediaObjectRef) throw (KmsMediaServerException);
  void subscribe(std::string& _return, const KmsMediaObjectRef& mediaObjectRef,
		 const std::string& eventType, const std::string& handlerAddress,
		 const int32_t handlerPort)throw (KmsMediaServerException);
  void unsubscribe(const KmsMediaObjectRef& mediaObjectRef, const std::string& callbackToken)
		   throw (KmsMediaServerException);
  void sendCommand(KmsMediaCommandResult& _return, const KmsMediaObjectRef& mediaObjectRef, const KmsMediaCommand& command)
		   throw (KmsMediaServerException);
  void getParent(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaObjectRef)
		 throw (KmsMediaServerException);
  void getMediaPipeline(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaObjectRef)
			throw (KmsMediaServerException);

  /* MediaPileline */
  void createMediaPipeline(KmsMediaObjectRef& _return) throw (KmsMediaServerException);
  void createMediaPipelineWithParams(KmsMediaObjectRef& _return, const KmsMediaParams& params)
				     throw (KmsMediaServerException);
  void createMediaElement(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaPipeline,
			  const std::string& elementType) throw (KmsMediaServerException);
  void createMediaElementWithParams(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaPipeline,
				    const std::string& elementType, const KmsMediaParams& params)
				    throw (KmsMediaServerException);
  void createMediaMixer(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaPipeline,
			const std::string& mixerType) throw (KmsMediaServerException);
  void createMediaMixerWithParams(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaPipeline,
				  const std::string& mixerType, const KmsMediaParams& params)
				  throw (KmsMediaServerException);

  /* MediaElement */
  void getMediaSrcs(std::vector<KmsMediaObjectRef> & _return, const KmsMediaObjectRef& mediaElement)
		    throw (KmsMediaServerException);
  void getMediaSinks(std::vector<KmsMediaObjectRef> & _return, const KmsMediaObjectRef& mediaElement)
		     throw (KmsMediaServerException);
  void getMediaSrcsByMediaType(std::vector<KmsMediaObjectRef> & _return, const KmsMediaObjectRef& mediaElement,
			       const KmsMediaType::type mediaType) throw (KmsMediaServerException);
  void getMediaSinksByMediaType(std::vector<KmsMediaObjectRef> & _return, const KmsMediaObjectRef& mediaElement,
				const KmsMediaType::type mediaType) throw (KmsMediaServerException);
  void getMediaSrcsByFullDescription(std::vector<KmsMediaObjectRef> & _return, const KmsMediaObjectRef& mediaElement,
				     const KmsMediaType::type mediaType, const std::string& description)
				     throw (KmsMediaServerException);
  void getMediaSinksByFullDescription(std::vector<KmsMediaObjectRef> & _return, const KmsMediaObjectRef& mediaElement,
				      const KmsMediaType::type mediaType, const std::string& description)
				      throw (KmsMediaServerException);

  /* MediaPad */
  void getMediaElement(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaPadRef) throw (KmsMediaServerException);

  /* MediaSrc */
  void connect(const KmsMediaObjectRef& mediaSrc, const KmsMediaObjectRef& mediaSink) throw (KmsMediaServerException);
  void disconnect(const KmsMediaObjectRef& mediaSrc, const KmsMediaObjectRef& mediaSink) throw (KmsMediaServerException);
  void getConnectedSinks(std::vector<KmsMediaObjectRef> & _return, const KmsMediaObjectRef& mediaSrc) throw (KmsMediaServerException);

  /* MediaSink */
  void getConnectedSrc(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mediaSinkRef) throw (KmsMediaServerException);

  /* Mixer */
  void createMixerEndPoint(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mixer) throw (KmsMediaServerException);
  void createMixerEndPointWithParams(KmsMediaObjectRef& _return, const KmsMediaObjectRef& mixer, const KmsMediaParams& params)
				     throw (KmsMediaServerException);

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
