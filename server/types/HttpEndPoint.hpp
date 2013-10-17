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

#ifndef __HTTP_END_POINT_HPP__
#define __HTTP_END_POINT_HPP__

#include "EndPoint.hpp"

namespace kurento
{

class HttpEndPoint : public EndPoint
{
public:
  HttpEndPoint (std::shared_ptr<MediaPipeline> parent, const std::map<std::string, KmsMediaParam>& params)
  throw (KmsMediaServerException);
  ~HttpEndPoint() throw ();

  std::string getUrl ();

  void invoke (KmsMediaInvocationReturn &_return, const std::string &command,
               const std::map<std::string, KmsMediaParam> & params) throw (KmsMediaServerException);

private:
  std::string url;
  bool urlSet = false;
  guint cookieLifetime;
  guint disconnectionTimeout;

  void setUrl (std::string);

private:
  void init (std::shared_ptr<MediaPipeline> parent, guint cookieLifetime, guint disconnectionTimeout)
  throw (KmsMediaServerException);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
  gulong actionRequestedHandlerId;
  gulong urlRemovedHandlerId;
  gulong urlExpiredHandlerId;

  friend gboolean register_http_end_point (gpointer data);
  friend void http_end_point_raise_petition_event (HttpEndPoint *httpEp);
};

} // kurento

#endif /* __HTTP_END_POINT_HPP__ */
