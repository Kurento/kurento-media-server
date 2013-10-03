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
  SdpEndPoint (std::shared_ptr<MediaObjectImpl> parent, const std::string type);
  virtual ~SdpEndPoint() throw () = 0;

  std::string generateOffer ();
  std::string processAnswer (const std::string &answer);
  std::string processOffer (const std::string &offer);
  std::string getLocalSessionDescription () throw (MediaServerException);
  std::string getRemoteSessionDescription () throw (MediaServerException);
};

} // kurento

#endif /* __SDP_END_POINT_HPP__ */
