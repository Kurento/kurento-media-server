/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef __MEDIA_OBJECT_IMPL_HPP__
#define __MEDIA_OBJECT_IMPL_HPP__

#include "mediaServer_types.h"
#include <gst/gst.h>

namespace kurento
{

class MediaObjectImpl : public MediaObjectRef
{
public:
  MediaObjectImpl();
  MediaObjectImpl (std::shared_ptr<MediaObjectImpl> parent);
  virtual ~MediaObjectImpl() throw () = 0;

  std::shared_ptr<MediaObjectImpl> getParent () throw (MediaServerException);
  virtual CommandResult sendCommand (const Command &command) throw (MediaServerException);

public:
  std::shared_ptr<MediaObjectImpl> parent;

protected:
  static Params defaultParams;
};

} // kurento

#endif /* __MEDIA_OBJECT_IMPL_HPP__ */
