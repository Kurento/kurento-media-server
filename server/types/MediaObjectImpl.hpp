/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#include <gst/gst.h>
#include <memory>

#include <MediaObject.hpp>

namespace kurento
{

class MediaObjectImpl : public virtual MediaObject
{

public:
  MediaObjectImpl (std::shared_ptr<MediaObjectImpl> parent);
  MediaObjectImpl ();

  virtual std::shared_ptr<MediaPipeline> getMediaPipeline() {
    return getParent()->getMediaPipeline();
  }

  std::shared_ptr<MediaObject> getParent() {
    return parent;
  }

  void release() {
    // TODO:
  }

  std::string getId ();

private:
  std::shared_ptr<MediaObjectImpl> parent;
  std::string id = createId();

  static std::string createId();
};

} /* kurento */

#endif /* __MEDIA_OBJECT_IMPL_HPP__ */

