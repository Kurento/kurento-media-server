/*
 * HttpEndPoint.cpp - Kurento Media Server
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

#include "HttpEndPoint.hpp"

namespace kurento
{

HttpEndPoint::HttpEndPoint (std::shared_ptr<MediaPipeline> parent) : EndPoint (parent)
{
  this->type.__set_endPoint (EndPointType::type::HTTP_END_POINT);
}

HttpEndPoint::~HttpEndPoint() throw ()
{
  this->url = "DUMMY URL";
}

std::string
HttpEndPoint::getUrl ()
{
  return url;
}

} // kurento
