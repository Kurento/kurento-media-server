#ifndef MIXER_IMPL
#define MIXER_IMPL

#include "mixer_types.h"
#include "types/JoinableImpl.h"

using namespace ::com::kurento::kms::api;

namespace com { namespace kurento { namespace kms {

	class MixerImpl : public Mixer, public virtual JoinableImpl {
	public:
		MixerImpl(MediaSession &session, const std::vector<MixerConfig::type> &config);
		~MixerImpl() throw();

	};

}}} // com::kurento::kms

#endif /* MIXER_IMPL */