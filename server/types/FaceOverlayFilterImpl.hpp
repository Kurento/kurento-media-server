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

#ifndef __FACE_OVERLAY_FILTER_IMPL_HPP__
#define __FACE_OVERLAY_FILTER_IMPL_HPP__

#include "FilterImpl.hpp"
#include <generated/FaceOverlayFilter.hpp>

namespace kurento
{

class FaceOverlayFilterImpl: public virtual FaceOverlayFilter, public FilterImpl
{
public:
  FaceOverlayFilterImpl (std::shared_ptr<MediaObjectImpl> parent,
                         int garbagePeriod);
  virtual ~FaceOverlayFilterImpl() throw () {};

  virtual void unsetOverlayedImage ();
  virtual void setOverlayedImage (const std::string &uri, float offsetXPercent,
                                  float offsetYPercent, float widthPercent, float heightPercent);

private:

  GstElement *faceOverlay = NULL;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} /* kurento */

#endif /* __FACE_OVERLAY_FILTER_IMPL_HPP__ */
