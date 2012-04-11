#include "types/MediaSessionImpl.h"

#include <glibmm.h>
#include <uuid/uuid.h>
#include <log.h>
#include <mediaObject_constants.h>
#include <managers/MediaSessionManager.h>
#include <mediaSession_constants.h>

using ::com::kurento::kms::MediaSessionImpl;
using ::com::kurento::kms::NetworkConnectionManager;
using ::com::kurento::kms::MixerManager;
using ::com::kurento::kms::MediaSessionManager;
using namespace ::com::kurento::kms::api;
using ::com::kurento::log::Log;

static Log l("MediaSessionImpl");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

bool MediaSessionImpl::pingTimeout() {
	w("Timeout on session %d, deleting.", object.id);
	MediaSessionManager::getInstance()->deleteMediaSession(this->object);
	return false;
}

MediaSessionImpl::MediaSessionImpl() : MediaObjectImpl(), MediaSession() {
	uuid_t uuid;
	char *uuid_str;

	__set_object(*this);

	uuid_str = (char *) g_malloc(sizeof(char) * 37);
	uuid_generate(uuid);
	uuid_unparse(uuid, uuid_str);
	std::string tk;
	tk.append(uuid_str);
	g_free(uuid_str);
	object.__set_token(tk);

	ncManager = new NetworkConnectionManager();
	mixerManager = new MixerManager();

	conn = Glib::signal_timeout().connect_seconds(
		sigc::mem_fun(*this, &MediaSessionImpl::pingTimeout),
		g_mediaSession_constants.DEFAULT_TIMEOUT);
}

MediaSessionImpl::~MediaSessionImpl() throw() {
	conn.disconnect();
	delete ncManager;
}

NetworkConnection&
MediaSessionImpl::createNetworkConnection(
		const std::vector<NetworkConnectionConfig::type>& config) {
	NetworkConnection &nc = ncManager->createNewtorkConnection(*this, config);
	return nc;
}

void
MediaSessionImpl::deleteNetworkConnection(
				const NetworkConnection& networkConnection) {
	ncManager->deleteNetworkConnection(networkConnection);
}

void
MediaSessionImpl::getNetworkConnections(std::vector<NetworkConnection> &_return) {
	ncManager->getNetworkConnections(_return);
}


Mixer&
MediaSessionImpl::createMixer(const std::vector<MixerConfig::type>& config) {
	return mixerManager->createMixer(*this, config);
}

void
MediaSessionImpl::deleteMixer(const Mixer& mixer) {
	return mixerManager->deleteMixer(mixer);
}


void
MediaSessionImpl::getMixers(std::vector<Mixer> &_return) {
	mixerManager->getMixers(_return);
}

void
MediaSessionImpl::ping(const int32_t timeout) {
	conn.disconnect();
	conn = Glib::signal_timeout().connect_seconds(
		sigc::mem_fun(*this, &MediaSessionImpl::pingTimeout), timeout);
}
