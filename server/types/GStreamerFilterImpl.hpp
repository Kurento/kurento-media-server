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

#ifndef __GSTREAMER_FILTER_IMPL_HPP__
#define __GSTREAMER_FILTER_IMPL_HPP__

#include "FilterImpl.hpp"
#include <generated/GStreamerFilter.hpp>

namespace kurento
{

class GStreamerFilterImpl: public virtual GStreamerFilter, public FilterImpl
{
public:
  GStreamerFilterImpl (const std::string &command,
                       std::shared_ptr<MediaObjectImpl> parent, int garbagePeriod);
  virtual ~GStreamerFilterImpl() throw () {};

private:
  GstElement *filter = NULL;

  void setCommandProperties (const std::string &rest_token);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} /* kurento */

#endif /* __GSTREAMER_FILTER_IMPL_HPP__ */
