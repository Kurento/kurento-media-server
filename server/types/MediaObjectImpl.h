#ifndef MEDIA_OBJECT_IMPL
#define MEDIA_OBJECT_IMPL

#include "mediaObject_types.h"

using ::com::kurento::kms::api::MediaObject;

namespace com { namespace kurento { namespace kms {

class MediaObjectImpl : public MediaObject  {
public:
	MediaObjectImpl();
	~MediaObjectImpl() throw() {};

	void ping(const int32_t timeout);
};

}}} // com::kurento::kms

#endif /* MEDIA_OBJECT_IMPL */