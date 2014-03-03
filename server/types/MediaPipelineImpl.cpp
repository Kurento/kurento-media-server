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

#include "MediaPipelineImpl.hpp"

namespace kurento
{

static void
bus_message_adaptor (GstBus *bus, GstMessage *message, gpointer data)
{
  auto func = reinterpret_cast<std::function<void (GstMessage *message) >*>
              (data);

  (*func) (message);
}

MediaPipelineImpl::MediaPipelineImpl (int garbagePeriod) :
  MediaObjectImpl (garbagePeriod)
{
  GstBus *bus;

  pipeline = gst_pipeline_new (NULL);
  g_object_set (G_OBJECT (pipeline), "async-handling", TRUE, NULL);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  busMessageLambda = [&] (GstMessage * message) {
    switch (message->type) {
    case GST_MESSAGE_ERROR: {
      GError *err = NULL;

      GST_ERROR ("Error on bus: %" GST_PTR_FORMAT, message);
      gst_debug_bin_to_dot_file_with_ts (GST_BIN (pipeline),
                                         GST_DEBUG_GRAPH_SHOW_ALL, "error");
      gst_message_parse_error (message, &err, NULL);
      std::string errorMessage (err->message);
      Error error (shared_from_this(), errorMessage , 0,
                   "UNEXPECTED_PIPELINE_ERROR");
      g_error_free (err);
      signalError (error);
      break;
    }

    default:
      break;
    }
  };

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline) );
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message", G_CALLBACK (bus_message_adaptor),
                    &busMessageLambda);
  g_object_unref (bus);
}

MediaPipelineImpl::~MediaPipelineImpl ()
{
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline) );

  gst_bus_remove_signal_watch (bus);
  g_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_object_unref (pipeline);
}

std::shared_ptr< MediaObject >
MediaPipeline::Factory::createObject (int garbagePeriod)
{
  std::shared_ptr<MediaObject> pipe (new MediaPipelineImpl (garbagePeriod) );
  return pipe;
}

} /* kurento */
