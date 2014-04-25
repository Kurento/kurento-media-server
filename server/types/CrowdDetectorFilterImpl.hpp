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

#ifndef __CROWD_DETECTOR_FILTER_IMPL_HPP__
#define __CROWD_DETECTOR_FILTER_IMPL_HPP__

#include "FilterImpl.hpp"
#include <CrowdDetectorFilter.hpp>

namespace kurento
{

class CrowdDetectorFilterImpl: public virtual CrowdDetectorFilter,
  public FilterImpl
{
public:
  CrowdDetectorFilterImpl (const
                           std::vector<std::shared_ptr<RegionOfInterest>> &rois,
                           std::shared_ptr<MediaObjectImpl> parent);
  virtual ~CrowdDetectorFilterImpl() throw ();

private:

  GstElement *crowdDetector;
  gulong bus_handler_id;

  std::function<void (GstMessage *) > busMessageLambda;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} /* kurento */

#endif /* __CROWD_DETECTOR_FILTER_IMPL_HPP__ */
