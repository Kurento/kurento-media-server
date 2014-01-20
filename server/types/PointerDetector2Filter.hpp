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

#ifndef __POINTER_DETECTOR2_FILTER_HPP__
#define __POINTER_DETECTOR2_FILTER_HPP__

#include "Filter.hpp"
#include "KmsMediaPointerDetector2FilterType_types.h"

namespace kurento
{

class PointerDetector2Filter : public Filter
{
public:
  PointerDetector2Filter (MediaSet &mediaSet,
                          std::shared_ptr<MediaPipeline> parent,
                          const std::map<std::string, KmsMediaParam> &params);

  ~PointerDetector2Filter() throw ();

  void invoke (KmsMediaInvocationReturn &_return, const std::string &command,
               const std::map<std::string, KmsMediaParam> &params)
  throw (KmsMediaServerException);

private:

  gulong bus_handler_id;

  GstElement *pointerDetector2;

  void raiseEvent (const std::string &type, const std::string &windowID);
  void addWindow (KmsMediaPointerDetectorWindow windowInfo);
  void removeWindow (std::string id);
  void calibrateColorToTrack ();
  void clearWindows();
  void subscribe (std::string &_return, const std::string &eventType,
                  const std::string &handlerAddress,
                  const int32_t handlerPort)
  throw (KmsMediaServerException);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

  friend void pointerDetector2_receive_message (GstBus *bus, GstMessage *message,
      gpointer element);
};

} // kurento

#endif /* __POINTER_DETECTOR2_FILTER_HPP__ */
