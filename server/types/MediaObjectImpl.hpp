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

#include <generated/MediaObject.hpp>

namespace kurento
{

class MediaObjectImpl : public virtual MediaObject
{

public:
  typedef enum {
    REF,
    UNREF
  } State;

  MediaObjectImpl (std::shared_ptr<MediaObjectImpl> parent, int garbagePeriod);
  MediaObjectImpl (int garbagePeriod);

  guint64 getId() {
    return id;
  }

  virtual std::shared_ptr<MediaPipeline> getMediaPipeline() {
    return getParent()->getMediaPipeline();
  }

  std::shared_ptr<MediaObject> getParent() {
    return parent;
  }

  void release() {
    // TODO:
  }

  int getGarbagePeriod() {
    return garbagePeriod;
  }

  State getState () {
    return state;
  }
  void setState (State state) {
    this->state = state;
  }

  virtual bool getUnregChilds() {
    return true;
  };

  std::string getIdStr ();

private:
  State state = REF;
  std::shared_ptr<MediaObjectImpl> parent;
  int garbagePeriod;
  guint64 id = createId();

  static guint64 createId();
};

} /* kurento */

#endif /* __MEDIA_OBJECT_IMPL_HPP__ */

