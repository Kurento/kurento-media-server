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

#ifndef __POINTER_DETECTOR_FILTER_HPP__
#define __POINTER_DETECTOR_FILTER_HPP__

#include "Filter.hpp"

namespace kurento
{

class PointerDetectorFilter : public Filter
{
public:
   PointerDetectorFilter(std::shared_ptr<MediaPipeline> parent, const std::map<std::string, KmsMediaParam>& params);
  ~PointerDetectorFilter() throw ();

private:

  gulong bus_handler_id;

  GstElement *pointerDetector;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend void pointerDetector_receive_message (GstBus *bus, GstMessage *message, gpointer element);
};

} // kurento

#endif /* __POINTER_DETECTOR_FILTER_HPP__ */
