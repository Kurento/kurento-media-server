/*
 * Stream.cpp - Kurento Media Server
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

#include "Stream.hpp"

namespace kurento
{

Stream::Stream (std::shared_ptr<MediaFactory> parent) : Joinable (parent)
{

}

Stream::~Stream() throw ()
{

}

void
Stream::generateOffer (std::string &_return)
{
  _return.assign ("SessionDescriptor test from generateOffer");
}

void
Stream::processAnswer (std::string &_return, const std::string &answer)
{
  _return.assign ("SessionDescriptor test from processAnswer");
}

void
Stream::processOffer (std::string &_return, const std::string &offer)
{
  _return.assign ("SessionDescriptor test from processOffer");
}

void
Stream::getLocalDescriptor (std::string &_return)
{
  _return.assign ("Local SessionDescriptor");
}

void
Stream::getRemoteDescriptor (std::string &_return)
{
  _return.assign ("Remote SessionDescriptor");
}

} // kurento
