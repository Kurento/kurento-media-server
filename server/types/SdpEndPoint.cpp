/*
 * SdpEndPoint.cpp - Kurento Media Server
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

#include "SdpEndPoint.hpp"

namespace kurento
{

SdpEndPoint::SdpEndPoint (std::shared_ptr<MediaObjectImpl> parent, SdpEndPointType::type type) : EndPoint (parent)
{
  this->type.__set_sdpEndPoint (type);
}

SdpEndPoint::~SdpEndPoint() throw ()
{

}

std::string
SdpEndPoint::generateOffer ()
{
  return "Session Description test from generateOffer";
}

std::string
SdpEndPoint::processAnswer (const std::string &answer)
{
  return "Session Description test from processAnswer";
}

std::string
SdpEndPoint::processOffer (const std::string &offer)
{
  return "Session Description test from processOffer";
}

std::string
SdpEndPoint::getLocalSessionDescription ()
{
  return "Local Session Description";
}

std::string
SdpEndPoint::getRemoteSessionDescription ()
{
  return "Remote Session Description";
}

} // kurento
