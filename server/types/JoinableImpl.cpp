#include "types/JoinableImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaObjectImpl;
using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::api::Joinable;

JoinableImpl::JoinableImpl() : Joinable(), MediaObjectImpl() {
	__set_parent((MediaObjectImpl) *this);
}
