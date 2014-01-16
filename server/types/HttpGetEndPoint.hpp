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

#ifndef __HTTP_GET_END_POINT_HPP__
#define __HTTP_GET_END_POINT_HPP__

#include "HttpEndPoint.hpp"

namespace kurento
{

class HttpGetEndPoint : public HttpEndPoint
{
public:
  HttpGetEndPoint (MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
                   const std::map<std::string, KmsMediaParam> &params)
  throw (KmsMediaServerException);
  ~HttpGetEndPoint() throw ();

  void subscribe (std::string &_return, const std::string &eventType,
                  const std::string &handlerAddress, const int32_t handlerPort)
  throw (KmsMediaServerException);

private:
  void init (bool terminateOnEOS, KmsMediaProfile profile)
  throw (KmsMediaServerException);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __HTTP_GET_END_POINT_HPP__ */