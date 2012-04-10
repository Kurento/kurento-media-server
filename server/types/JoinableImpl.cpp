#include "types/JoinableImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaObjectImpl;
using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::Joinable;

JoinableImpl::JoinableImpl(MediaSession &session) :
					MediaObjectImpl(session.object.token),
					Joinable() {
	__set_object(*this);
	__set_session(session);
}
