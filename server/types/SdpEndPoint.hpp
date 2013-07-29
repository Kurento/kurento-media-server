/*
 * SdpEndPoint.hpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SDP_END_POINT_HPP__
#define __SDP_END_POINT_HPP__

#include "EndPoint.hpp"

namespace kurento
{

class SdpEndPoint : public EndPoint
{
public:
  SdpEndPoint (std::shared_ptr<MediaObjectImpl> parent);
  ~SdpEndPoint() throw ();

  std::string generateOffer ();
  std::string processAnswer (const std::string &answer);
  std::string processOffer (const std::string &offer);
  std::string getLocalSessionDescription ();
  std::string getRemoteSessionDescription ();
};

} // kurento

#endif /* __SDP_END_POINT_HPP__ */
