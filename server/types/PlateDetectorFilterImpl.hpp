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

#ifndef __PLATE_DETECTOR_FILTER_IMPL_HPP__
#define __PLATE_DETECTOR_FILTER_IMPL_HPP__

#include "FilterImpl.hpp"
#include <generated/PlateDetectorFilter.hpp>

namespace kurento
{

class PlateDetectorFilterImpl: public virtual PlateDetectorFilter,
  public FilterImpl
{
public:
  PlateDetectorFilterImpl (std::shared_ptr<MediaObjectImpl> parent,
                           int garbagePeriod);
  virtual ~PlateDetectorFilterImpl() throw ();

private:

  gulong bus_handler_id;
  GstElement *plateDetector = NULL;
  std::function<void (GstMessage *) > busMessageLambda;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} /* kurento */

#endif /* __PLATE_DETECTOR_FILTER_IMPL_HPP__ */
