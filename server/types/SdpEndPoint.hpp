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

#ifndef __SDP_END_POINT_HPP__
#define __SDP_END_POINT_HPP__

#include "EndPoint.hpp"

namespace kurento
{

class SdpEndPoint : public EndPoint
{
public:
  SdpEndPoint (MediaSet &mediaSet, std::shared_ptr<MediaObjectImpl> parent,
               const std::string &type,
               const std::map<std::string, KmsMediaParam> &params,
               const std::string &factoryName);
  virtual ~SdpEndPoint() throw () = 0;

  void generateOffer (std::string &_return);
  void processAnswer (std::string &_return, const std::string &answer);
  void processOffer (std::string &_return, const std::string &offer);
  void getLocalSessionDescription (std::string &_return) throw (
    KmsMediaServerException);
  void getRemoteSessionDescription (std::string &_return) throw (
    KmsMediaServerException);
  void subscribe (std::string &_return, const std::string &eventType,
                  const std::string &handlerAddress,
                  const int32_t handlerPort) throw (KmsMediaServerException);

  void invoke (KmsMediaInvocationReturn &_return, const std::string &command,
               const std::map<std::string, KmsMediaParam> &params) throw (
                 KmsMediaServerException);

private:

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __SDP_END_POINT_HPP__ */
