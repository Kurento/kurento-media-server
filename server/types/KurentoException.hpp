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

/* Error codes */
/* GENERIC MEDIA ERRORS */
#define MEDIA_ERROR_MIN 40000
#define MEDIA_ERROR_MAX 40099
// #define MEDIA_ERROR 40000
// #define MARSHALL_ERROR 40001
// #define UNMARSHALL_ERROR 40002
// #define UNEXPECTED_ERROR 40003
#define CONNECT_ERROR 40004
// #define UNSUPPORTED_MEDIA_TYPE 40005
// #define NOT_IMPLEMENTED 40006
#define INVALID_SESSION 40007

/* MediaObject ERRORS */
#define MEDIA_OBJECT_ERROR_MIN 40100
#define MEDIA_OBJECT_ERROR_MAX 40199
#define MEDIA_OBJECT_TYPE_NOT_FOUND 40100
#define MEDIA_OBJECT_NOT_FOUND 40101
// #define MEDIA_OBJECT_CAST_ERROR 40102
// #define MEDIA_OBJECT_HAS_NOT_PARENT 40103
// #define MEDIA_OBJECT_CONSTRUCTOR_NOT_FOUND 40104
// #define MEDIA_OBJECT_METHOD_NOT_FOUND 40105
#define MEDIA_OBJECT_EVENT_NOT_SUPPORTED 40106
#define MEDIA_OBJECT_ILLEGAL_PARAM_ERROR 40107
#define MEDIA_OBJECT_NOT_AVAILABLE 40107

/* SDP ERRORS */
#define SDP_ERROR_MIN 40200
#define SDP_ERROR_MAX 40299
#define SDP_CREATE_ERROR 40200
#define SDP_PARSE_ERROR 40201
#define SDP_END_POINT_NO_LOCAL_SDP_ERROR 40202
#define SDP_END_POINT_NO_REMOTE_SDP_ERROR 40203
#define SDP_END_POINT_GENERATE_OFFER_ERROR 40204
#define SDP_END_POINT_PROCESS_OFFER_ERROR 40205
// #define SDP_END_POINT_PROCESS_ANSWER_ERROR 40206

/* HTTP ERRORS */
#define HTTP_ERROR_MIN 40300
#define HTTP_ERROR_MAX 40399
#define HTTP_END_POINT_REGISTRATION_ERROR 40300

namespace kurento
{

class KurentoException: public virtual std::exception
{
public:
  KurentoException (int code, const std::string &message) : message (message),
    code (code) {};
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
