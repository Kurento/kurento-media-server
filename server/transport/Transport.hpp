/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
