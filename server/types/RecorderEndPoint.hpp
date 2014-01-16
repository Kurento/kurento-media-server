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

#ifndef __RECORDER_END_POINT_HPP__
#define __RECORDER_END_POINT_HPP__

#include "UriEndPoint.hpp"
#include "KmsMediaProfile_types.h"

namespace kurento
{

class RecorderEndPoint : public UriEndPoint
{
public:
  RecorderEndPoint (MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
                    const std::map<std::string, KmsMediaParam> &params)
  throw (KmsMediaServerException);
  ~RecorderEndPoint() throw ();

private:
  void init (std::shared_ptr<MediaPipeline> parent, const std::string &uri,
             KmsMediaProfile profile);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __RECORDER_END_POINT_HPP__ */
