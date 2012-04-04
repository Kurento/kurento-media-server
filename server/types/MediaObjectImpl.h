#ifndef MEDIA_OBJECT_IMPL
#define MEDIA_OBJECT_IMPL

#include "mediaObject_types.h"
#include "MediaObjectService.h"

using ::com::kurento::kms::api::MediaObject;
using ::com::kurento::kms::api::MediaObjectServiceIf;

namespace com { namespace kurento { namespace kms {

class MediaObjectImpl : public MediaObject, virtual public MediaObjectServiceIf  {
public:
	MediaObjectImpl();
	~MediaObjectImpl() throw() {};

	void ping(const MediaObject& mediaObject, const int32_t timeout);
	void release(const MediaObject& mediaObject);
};

}}} // com::kurento::kms

#endif /* MEDIA_OBJECT_IMPL */