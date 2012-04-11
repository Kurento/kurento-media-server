#include "types/MixerImpl.h"

using ::com::kurento::kms::MixerImpl;
using ::com::kurento::kms::api::MixerConfig;

MixerImpl::MixerImpl(MediaSession &session,
			const std::vector<MixerConfig::type> & config) :
					JoinableImpl(session), Mixer() {
	__set_joinable(*this);

	__set_config(config);
}
