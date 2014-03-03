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

#ifndef __KURENTO_EXCEPTION_HPP__
#define __KURENTO_EXCEPTION_HPP__

#include <exception>
#include <string>

namespace kurento
{

class KurentoException: public virtual std::exception
{
public:
  KurentoException (const std::string &message) : message (message), code (0) {};
  virtual ~KurentoException() {};

  virtual const char *what() const noexcept {
    return message.c_str();
  };

  const std::string &getMessage() const {
    return message;
  };

  int getCode() const {
    return code;
  }

private:

  std::string message;
  int code;

};

} /* kurento */

#endif /* __KURENTO_EXCEPTION_HPP__ */
