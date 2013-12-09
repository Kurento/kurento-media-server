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

#ifndef __FACE_OVERLAY_FILTER_HPP__
#define __FACE_OVERLAY_FILTER_HPP__

#include "Filter.hpp"
#include "KmsMediaFaceOverlayFilterType_types.h"

namespace kurento
{

class FaceOverlayFilter : public Filter
{
public:
  FaceOverlayFilter (MediaSet &mediaSet,
                         std::shared_ptr<MediaPipeline> parent,
                         const std::map<std::string, KmsMediaParam>& params);

  ~FaceOverlayFilter() throw ();

  GstElement *faceOverlay;

  void invoke (KmsMediaInvocationReturn &_return, const std::string &command,
               const std::map<std::string, KmsMediaParam> & params)
               throw (KmsMediaServerException);

private:

  void setImageOverlay(KmsMediaFaceOverlayImage image);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __FACE_OVERLAY_FILTER_HPP__ */