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

#ifndef __RECORDER_ENDPOINT_IMPL_HPP__
#define __RECORDER_ENDPOINT_IMPL_HPP__

#include "UriEndpointImpl.hpp"
#include <generated/RecorderEndpoint.hpp>

namespace kurento
{

class RecorderEndpointImpl: public virtual RecorderEndpoint,
  public UriEndpointImpl
{
public:
  RecorderEndpointImpl (std::shared_ptr<MediaProfileSpecType> mediaProfile,
                        bool stopOnEndOfStream, const std::string &uri,
                        std::shared_ptr< MediaObjectImpl > parent,
                        int garbagePeriod);
  virtual ~RecorderEndpointImpl() throw ();

  void record();

private:

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __RECORDER_ENDPOINT_IMPL_HPP__ */
