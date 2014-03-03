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

#ifndef __JACK_VADER_FILTER_IMPL_HPP__
#define __JACK_VADER_FILTER_IMPL_HPP__

#include "FilterImpl.hpp"
#include <generated/JackVaderFilter.hpp>

namespace kurento
{

class JackVaderFilterImpl: public virtual JackVaderFilter, public FilterImpl
{
public:
  JackVaderFilterImpl (std::shared_ptr<MediaObjectImpl> parent,
                       int garbagePeriod);
  virtual ~JackVaderFilterImpl() throw () {};

private:

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} /* kurento */

#endif /* __JACK_VADER_FILTER_IMPL_HPP__ */
