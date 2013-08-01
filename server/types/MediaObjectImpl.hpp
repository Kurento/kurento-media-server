/*
 * MediaObjectImpl.hpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MEDIA_OBJECT_IMPL_HPP__
#define __MEDIA_OBJECT_IMPL_HPP__

#include "mediaServer_types.h"
#include <gst/gst.h>

namespace kurento
{

class MediaObjectImpl : public MediaObject
{
public:
  MediaObjectImpl();
  MediaObjectImpl (std::shared_ptr<MediaObject> parent);
  virtual ~MediaObjectImpl() throw () = 0;

  std::shared_ptr<MediaObject> getParent () throw (NoParentException);
  gchar * getIdStr ();

public:
  std::shared_ptr<MediaObject> parent;
  GstElement *element;
};

} // kurento

#endif /* __MEDIA_OBJECT_IMPL_HPP__ */