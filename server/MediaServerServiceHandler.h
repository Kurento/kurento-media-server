/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MEDIA_SERVER_SERVICE_HANDLER
#define MEDIA_SERVER_SERVICE_HANDLER

#include "MediaServerService.h"

using ::kurento::MediaServerServiceIf;

namespace kurento {

class MediaServerServiceHandler : public MediaServerServiceIf {
public:
	MediaServerServiceHandler();
	~MediaServerServiceHandler();

	void ping(const MediaObject& resource, const int32_t timeout);
	void release(const MediaObject& mediaObject);

	void createMediaFactory(MediaObject& _return);
	void createMediaPlayer(MediaObject& _return, const MediaObject& mediaFactory);
	void createMediaRecorder(MediaObject& _return, const MediaObject& mediaFactory);
	void createStream(MediaObject& _return, const MediaObject& mediaFactory);
	void createMixer(MediaObject& _return, const MediaObject& mediaFactory, const int32_t mixerId);

	MediaType::type getMediaType(const MediaObject& mediaElement);
	void connect(const MediaObject& mediaSrc, const MediaObject& mediaSink);
	void disconnect(const MediaObject& src, const MediaObject& mediaSink);
	void getConnectedSinks(std::vector<MediaObject> & _return, const MediaObject& mediaSrc);
	void getConnectedSrc(MediaObject& _return, const MediaObject& mediaSink);

	void join(const MediaObject& joinableA, const MediaObject& joinableB);
	void unjoin(const MediaObject& joinableA, const MediaObject& joinableB);
	void getMediaSrcs(std::vector<MediaObject> & _return, const MediaObject& joinable);
	void getMediaSinks(std::vector<MediaObject> & _return, const MediaObject& joinable);
	void getMediaSrcsByMediaType(std::vector<MediaObject> & _return, const MediaObject& joinable, const MediaType::type mediaType);
	void getMediaSinksByMediaType(std::vector<MediaObject> & _return, const MediaObject& joinable, const MediaType::type mediaType);

	void play(const MediaObject& mediaPlayer);
	void pausePlayer(const MediaObject& mediaPlayer);
	void stopPlayer(const MediaObject& mediaPlayer);

	void record(const MediaObject& mediaRecorder);
	void pauseRecorder(const MediaObject& mediaRecorder);
	void stopRecorder(const MediaObject& mediaRecorder);

	void generateOffer(std::string& _return, const MediaObject& stream);
	void processAnswer(std::string& _return, const MediaObject& stream, const std::string& answer);
	void processOffer(std::string& _return, const MediaObject& stream, const std::string& offer);
	void getLocalDescriptor(std::string& _return, const MediaObject& stream);
	void getRemoteDescriptor(std::string& _return, const MediaObject& stream);

	void getMixerPort(MediaObject& _return, const MediaObject& mixer);
	void getMixer(MediaObject& _return, const MediaObject& mixerPort);
};

} // kurento

#endif /* MEDIA_SERVER_SERVICE_HANDLER */
