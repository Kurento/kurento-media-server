#ifndef MIXER_SERVICE_HANDLER
#define MIXER_SERVICE_HANDLER

#include "MixerService.h"
#include "JoinableServiceHandler.h"
#include "managers/MixerManager.h"

using ::com::kurento::kms::api::MixerServiceIf;

namespace com { namespace kurento { namespace kms {

class MixerServiceHandler : virtual public MixerServiceIf,
				virtual public JoinableServiceHandler {
public:
	MixerServiceHandler();
	~MixerServiceHandler();

};

}}} // com::kurento::kms::api

#endif /* MIXER_SERVICE_HANDLER */
