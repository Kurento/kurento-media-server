#include "types/MediaSessionImpl.h"

#include <glibmm.h>
#include <uuid/uuid.h>
#include <log.h>
#include <managers/MediaSessionManager.h>
#include <mediaSession_constants.h>

using ::com::kurento::kms::MediaSessionImpl;
using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
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
	MediaSessionManager::getInstance()->deleteMediaSession(*this);
	return false;
}

MediaSessionImpl::MediaSessionImpl(const SessionSpec &spec) :
							MediaObjectImpl(),
							MediaSession(),
							ncManager(spec) {
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

	conn = Glib::signal_timeout().connect_seconds(
		sigc::mem_fun(*this, &MediaSessionImpl::pingTimeout),
		g_mediaSession_constants.DEFAULT_TIMEOUT);
}

MediaSessionImpl::~MediaSessionImpl() throw() {
	conn.disconnect();
}

NetworkConnection&
MediaSessionImpl::createNetworkConnection(
		const std::vector<NetworkConnectionConfig::type>& config) {
	NetworkConnection &nc = ncManager.createNewtorkConnection(*this, config);
	return nc;
}

void
MediaSessionImpl::deleteNetworkConnection(
				const NetworkConnection& networkConnection) {
	ncManager.deleteNetworkConnection(networkConnection);
}

void
MediaSessionImpl::getNetworkConnections(std::vector<NetworkConnection> &_return) {
	ncManager.getNetworkConnections(_return);
}


Mixer&
MediaSessionImpl::createMixer(const std::vector<MixerConfig::type>& config) {
	return mixerManager.createMixer(*this, config);
}

void
MediaSessionImpl::deleteMixer(const Mixer& mixer) {
	return mixerManager.deleteMixer(mixer);
}


void
MediaSessionImpl::getMixers(std::vector<Mixer> &_return) {
	mixerManager.getMixers(_return);
}

void
MediaSessionImpl::ping(const int32_t timeout) {
	conn.disconnect();
	conn = Glib::signal_timeout().connect_seconds(
		sigc::mem_fun(*this, &MediaSessionImpl::pingTimeout), timeout);
}

JoinableImpl &
MediaSessionImpl::getJoinable(const Joinable &joinable) {
	try {
		return ncManager.getJoinable(joinable);
	} catch (JoinableNotFoundException e) {
		return mixerManager.getJoinable(joinable);
	}
}

void
MediaSessionImpl::deleteJoinable(const Joinable &joinable) {
	try {
		ncManager.deleteJoinable(joinable);
	} catch (JoinableNotFoundException e) {
		mixerManager.deleteJoinable(joinable);
	}
}

NetworkConnectionImpl&
MediaSessionImpl::getNetworkConnection(const NetworkConnection &nc) {
	return ncManager.getNetworkConnection(nc);
}
