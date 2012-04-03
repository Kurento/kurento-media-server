#ifndef MEDIA_SESSION_IMPL
#define MEDIA_SESSION_IMPL

#include "mediaSession_types.h"
#include "types/MediaObjectImpl.h"

using ::com::kurento::kms::api::MediaSession;

namespace com { namespace kurento { namespace kms {

class MediaSessionImpl : public MediaSession {
public:
	MediaSessionImpl();
	~MediaSessionImpl() throw() {};
};

}}} // com::kurento::kms

#endif /* MEDIA_SESSION_IMPL */