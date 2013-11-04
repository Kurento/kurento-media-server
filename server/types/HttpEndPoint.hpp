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
#include "httpendpointserver.hpp"

namespace kurento
{

class HttpEndPoint : public EndPoint
{
public:
  HttpEndPoint (MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
                const std::map<std::string, KmsMediaParam>& params)
  throw (KmsMediaServerException);
  ~HttpEndPoint() throw ();

  std::string getUrl ();

  void invoke (KmsMediaInvocationReturn &_return, const std::string &command,
               const std::map<std::string, KmsMediaParam> & params) throw (KmsMediaServerException);
  void subscribe (std::string &_return, const std::string &eventType,
                  const std::string &handlerAddress, const int32_t handlerPort) throw (KmsMediaServerException);

private:
  std::string url;
  bool urlSet = false;
  guint disconnectionTimeout;

  void setUrl (const std::string &);

private:
  void init (std::shared_ptr<MediaPipeline> parent, guint disconnectionTimeout,
    bool terminateOnEOS) throw (KmsMediaServerException);

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
  gulong actionRequestedHandlerId;
  gulong urlRemovedHandlerId;
  gulong urlExpiredHandlerId;
  gint sessionStarted = 0;

  friend gboolean register_http_end_point (gpointer data);
  friend void http_end_point_raise_petition_event (HttpEndPoint *httpEp, KmsHttpEndPointAction action);
  friend void kurento_http_end_point_raise_session_terminated_event (HttpEndPoint *httpEp, const gchar *uri);
  friend void kurento_http_end_point_eos_detected_cb (GstElement *element, gpointer data);
};

} // kurento

#endif /* __HTTP_END_POINT_HPP__ */
