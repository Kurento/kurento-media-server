#include "MediaSessionServiceHandler.h"
#include "log.h"

using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaSessionHandler");
#define i(...) aux_info(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms {

MediaSessionServiceHandler::MediaSessionServiceHandler() {
	manager = MediaSessionManager::getInstance();
}

MediaSessionServiceHandler::~MediaSessionServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}

void
MediaSessionServiceHandler::createNetworkConnection(NetworkConnection& _return,
					const MediaSession& mediaSession,
					const std::vector<NetworkConnectionConfig::type>& config) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		_return = session.createNetworkConnection(config);

		i("Created NetworkConnection with id: %d", _return.joinable.object.id);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::deleteNetworkConnection(
				const MediaSession& mediaSession,
				const NetworkConnection& networkConnection) {
	i("deleteNetworkConnection: %d", networkConnection.joinable.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.deleteNetworkConnection(networkConnection);
	} catch(NetworkConnectionNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::getNetworkConnections(
					std::vector<NetworkConnection>& _return,
					const MediaSession& mediaSession) {
	i("getNetworkConnections");
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.getNetworkConnections(_return);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::createMixer(Mixer& _return,
				const MediaSession& mediaSession,
				const std::vector<MixerConfig::type>& config) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		_return = session.createMixer(config);
		i("Mixer created with id: %d", _return.joinable.object.id);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::deleteMixer(const MediaSession& mediaSession,
							const  Mixer& mixer) {
	i("deleteMixer with id: %d", mixer.joinable.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.deleteMixer(mixer);
	} catch(MixerNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::getMixers(std::vector<Mixer>& _return,
					const MediaSession& mediaSession) {
	i("getMixers");
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.getMixers(_return);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::ping(const MediaObject& mediaObject,
							const int32_t timeout) {
	i("ping");
}

void
MediaSessionServiceHandler::release(const MediaObject& mediaObject) {
	i("release");
}

}}} // com::kurento::kms