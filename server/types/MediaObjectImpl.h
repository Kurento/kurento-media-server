#ifndef MEDIA_OBJECT_IMPL
#define MEDIA_OBJECT_IMPL

#include "common_types.h"

using ::com::kurento::kms::api::MediaObject;

namespace com { namespace kurento { namespace kms {

class MediaObjectImpl : public virtual MediaObject  {
public:
	MediaObjectImpl();
	MediaObjectImpl(std::string &token);
	~MediaObjectImpl() throw() {};
};

}}} // com::kurento::kms

#endif /* MEDIA_OBJECT_IMPL */