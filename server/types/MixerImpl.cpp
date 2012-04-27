#include "types/MixerImpl.h"

#include <mixer/kms-mixer.h>

#include <log.h>

using ::com::kurento::kms::MixerImpl;
using ::com::kurento::kms::api::MixerConfig;

using ::com::kurento::log::Log;

static Log l("MixerImpl");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);


MixerImpl::MixerImpl(MediaSession &session,
			const std::vector<MixerConfig::type> & config) :
					JoinableImpl(session), Mixer() {
	__set_joinable(*this);

	__set_config(config);

	endpoint = KMS_ENDPOINT(g_object_new(KMS_TYPE_MIXER_ENDPOINT, NULL));

	if (endpoint == NULL) {
		MediaServerException ex;
		ex.__set_code(ErrorCode::NO_RESOURCES);
		ex.__set_description("Unable to create network connection");
		w(ex.description);
		throw ex;
	}
}

MixerImpl::~MixerImpl() throw () {
	g_object_unref(endpoint);
	endpoint = NULL;
}
