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

#include "HttpPostEndpointImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include <KurentoException.hpp>

#define USE_ENCODED_MEDIA "use-encoded-media"

#define GST_CAT_DEFAULT kurento_http_post_endpoint_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "HttpPOSTEndpointImpl"

namespace kurento
{

static void
adaptor_function (GstElement *player, gpointer data)
{
  auto handler = reinterpret_cast<std::function<void() >*> (data);

  (*handler) ();
}

HttpPostEndpointImpl::HttpPostEndpointImpl (
  bool useEncodedMedia, int disconnectionTimeout,
  std::shared_ptr< MediaObjectImpl > mediaPipeline) :
  HttpEndpointImpl (disconnectionTimeout, mediaPipeline)
{
  eosLambda = [&] () {
    EndOfStream event (shared_from_this(), EndOfStream::getName() );

    signalEndOfStream (event);
  };

  /* Do not accept EOS */
  g_object_set ( G_OBJECT (element), "accept-eos", false, NULL);
  g_signal_connect (element, "eos", G_CALLBACK (adaptor_function), this);

  register_end_point();

  if (!is_registered() ) {
    throw KurentoException (HTTP_END_POINT_REGISTRATION_ERROR,
                            "Cannot register HttpPostEndPoint");
  }

  g_object_set (G_OBJECT (element), USE_ENCODED_MEDIA, useEncodedMedia, NULL);
}

MediaObject  *
HttpPostEndpoint::Factory::createObject (
  std::shared_ptr< MediaPipeline > mediaPipeline, int disconnectionTimeout,
  bool useEncodedMedia)
{
  return new HttpPostEndpointImpl (useEncodedMedia, disconnectionTimeout,
                                   std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline) );
}

HttpPostEndpointImpl::StaticConstructor HttpPostEndpointImpl::staticConstructor;

HttpPostEndpointImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
