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

#ifndef __MARSHALLING_HPP__
#define __MARSHALLING_HPP__

#include "mediaServer_types.h"

namespace kurento
{

std::string marshalString (std::string str) throw (MediaServerException);
std::string unmarshalString (std::string data) throw (MediaServerException);

} // kurento

#endif /* __MARSHALLING_HPP__ */
