#include "types/MediaSessionImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaSessionImpl;

MediaSessionImpl::MediaSessionImpl() : MediaSession() {
	MediaObjectImpl parent;
	__set_parent(parent);
}
