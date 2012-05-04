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