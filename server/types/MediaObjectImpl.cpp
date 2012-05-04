/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

MediaObjectImpl::MediaObjectImpl(std::string &token) : MediaObject() {
	id = getId();
	this->token = token;
}
