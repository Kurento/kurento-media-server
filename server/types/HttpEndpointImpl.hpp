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

#ifndef __HTTP_ENDPOINT_IMPL_HPP__
#define __HTTP_ENDPOINT_IMPL_HPP__

#include "SessionEndpointImpl.hpp"
#include <generated/HttpEndpoint.hpp>
#include "httpendpointserver.hpp"
#include <httpendpointserver.hpp>

namespace kurento
{

class HttpEndpointImpl: public virtual HttpEndpoint, public SessionEndpointImpl
{
public:
  HttpEndpointImpl (int disconnectionTimeout,
                    std::shared_ptr< MediaObjectImpl > parent,
                    int garbagePeriod);
  virtual ~HttpEndpointImpl() throw ();

  virtual std::string getUrl ();

protected:
  void register_end_point ();
  bool is_registered();

private:
  std::string url;
  bool urlSet = false;
  guint disconnectionTimeout;

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

  std::function<void (const gchar *uri, KmsHttpEndPointAction action) >
  actionRequestedLambda;
  std::function<void (const gchar *uri) > sessionTerminatedLambda;
};

} /* kurento */

#endif /* __HTTP_ENDPOINT_IMPL_HPP__ */
