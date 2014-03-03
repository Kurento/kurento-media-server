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

#ifndef __CHROMA_FILTER_IMPL_HPP__
#define __CHROMA_FILTER_IMPL_HPP__

#include "FilterImpl.hpp"
#include <generated/ChromaFilter.hpp>

namespace kurento
{

class ChromaFilterImpl: public virtual ChromaFilter, public FilterImpl
{
public:
  ChromaFilterImpl (std::shared_ptr<WindowParam> window,
                    const std::string &backgroundImage,
                    std::shared_ptr<MediaObjectImpl> parent,
                    int garbagePeriod);
  virtual ~ChromaFilterImpl() throw () {};

  void setBackground (const std::string &uri);
  void unsetBackground ();

private:

  GstElement *chroma = NULL;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

};

} /* kurento */

#endif /* __CHROMA_FILTER_IMPL_HPP__ */
