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

#include "RecorderEndpointImpl.hpp"
#include <generated/MediaProfileSpecType.hpp>
#include <generated/MediaPipeline.hpp>

#define GST_CAT_DEFAULT kurento_recorder_endpoint_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRecorderEndpointImpl"

#define FACTORY_NAME "recorderendpoint"

namespace kurento
{

enum {
  WEBM = 0,
  MP4 = 1
};

RecorderEndpointImpl::RecorderEndpointImpl (
  std::shared_ptr<MediaProfileSpecType> mediaProfile,
  bool stopOnEndOfStream,
  const std::string &uri,
  std::shared_ptr< MediaObjectImpl > parent,
  int garbagePeriod) :
  UriEndpointImpl (uri, FACTORY_NAME,
                   parent, garbagePeriod)
{
  g_object_ref (getGstreamerElement() );

  g_object_set (G_OBJECT (getGstreamerElement() ), "accept-eos",
                stopOnEndOfStream, NULL);

  switch (mediaProfile->getValue() ) {
  case MediaProfileSpecType::WEBM:
    g_object_set ( G_OBJECT (element), "profile", WEBM, NULL);
    GST_INFO ("Set WEBM profile");
    break;

  case MediaProfileSpecType::MP4:
    g_object_set ( G_OBJECT (element), "profile", MP4, NULL);
    GST_INFO ("Set MP4 profile");
    break;
  }
}

static void
dispose_element (GstElement *element)
{
  GST_TRACE_OBJECT (element, "Disposing");

  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

static void
state_changed (GstElement *element, gint state, gpointer data)
{
  GST_TRACE_OBJECT (element, "State changed: %d", state);
  dispose_element (element);
}

RecorderEndpointImpl::~RecorderEndpointImpl()
{
  gint state = -1;

  g_object_get (getGstreamerElement(), "state", &state, NULL);

  if (state == 0 /* stop */) {
    dispose_element (getGstreamerElement() );
    return;
  }

  g_signal_connect (getGstreamerElement(), "state-changed",
                    G_CALLBACK (state_changed), NULL);

  stop();
}


void
RecorderEndpointImpl::record ()
{
  start();
}

std::shared_ptr<MediaObject>
RecorderEndpoint::Factory::createObject (std::shared_ptr<MediaPipeline>
    mediaPipeline, const std::string &uri,
    std::shared_ptr<MediaProfileSpecType> mediaProfile, bool stopOnEndOfStream,
    int garbagePeriod)
{
  std::shared_ptr<MediaObject> obj (
    new RecorderEndpointImpl (mediaProfile,
                              stopOnEndOfStream, uri,
                              std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline),
                              garbagePeriod) );

  return obj;
}


RecorderEndpointImpl::StaticConstructor RecorderEndpointImpl::staticConstructor;

RecorderEndpointImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
