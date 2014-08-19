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

#ifndef __TRANSPORT_HPP__
#define __TRANSPORT_HPP__

#include <boost/property_tree/ptree.hpp>

namespace kurento
{

class Transport
{
public:
  Transport () {};
  virtual ~Transport() throw () {};
  virtual void start () = 0;
  virtual void stop () = 0;
};

} /* kurento */

#endif /* __TRANSPORT_HPP__ */
