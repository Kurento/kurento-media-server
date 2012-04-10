#include "types/MediaObjectImpl.h"

#include <glibmm.h>

using ::com::kurento::kms::MediaObjectImpl;
using ::com::kurento::kms::api::ObjectId;
using ::com::kurento::kms::api::MediaObject;

static ObjectId
getId() {
	static Glib::Mutex mutex;
	static ObjectId count = 0;

	long ret;

	mutex.lock();
	ret = count ++;
	mutex.unlock();

	return ret;
}

MediaObjectImpl::MediaObjectImpl() : MediaObject() {
	id = getId();
}

MediaObjectImpl::MediaObjectImpl(std::string token) : MediaObject() {
	id = getId();
	this->token = token;
}
