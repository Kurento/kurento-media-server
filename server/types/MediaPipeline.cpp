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

#include "MediaPipeline.hpp"

#include "utils/utils.hpp"
#include "dataTypes_constants.h"
#include "errorCodes_constants.h"

#include "PlayerEndPointType_constants.h"
#include "PlayerEndPoint.hpp"

#define GST_CAT_DEFAULT kurento_media_pipeline
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaPipeline"

namespace kurento
{

static void
receive_message (GstBus *bus, GstMessage *message, gpointer pipeline)
{
  switch (message->type) {
  case GST_MESSAGE_ERROR:
    GST_ERROR ("Error on bus: %P", message);
    gst_debug_bin_to_dot_file_with_ts (GST_BIN (pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "error");
    // TODO: Check if further notification is needed
    break;
  default:
    break;
  }
}

void
MediaPipeline::init ()
{
  GstBus *bus;

  pipeline = gst_pipeline_new (NULL);
  g_object_set (G_OBJECT (pipeline), "async-handling", TRUE, NULL);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline) );
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message", G_CALLBACK (receive_message), pipeline);
  g_object_unref (bus);

  this->type.__set_pipelineType (*this);
}

MediaPipeline::MediaPipeline (const Params &params) throw (MediaServerException)
  : MediaObjectImpl (),
    MediaPipelineType ()
{
  if (params == defaultParams ||
      g_dataTypes_constants.VOID_DATA_TYPE.compare (params.dataType) == 0) {
    init ();
  } else {
    throw createMediaServerException (g_errorCodes_constants.MEDIA_OBJECT_CONSTRUCTOR_NOT_FOUND,
        "MediaPipeline has not any constructor with params of type " + params.dataType);
  }
}

MediaPipeline::~MediaPipeline() throw()
{
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline) );
  gst_bus_remove_signal_watch (bus);
  g_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_object_unref (pipeline);
}

std::shared_ptr<MediaElement>
MediaPipeline::createMediaElement (const std::string &elementType, const Params &params)
throw (MediaServerException)
{
  GST_WARNING ("TODO: complete");

  if (g_PlayerEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    return std::shared_ptr<PlayerEndPoint> (new PlayerEndPoint (shared_from_this (), params) );
  }

  throw createMediaServerException (g_errorCodes_constants.MEDIA_OBJECT_TYPE_NOT_FOUND,
      "There is not any media object type " + elementType);
}

std::shared_ptr<Mixer>
MediaPipeline::createMediaMixer (const std::string &mixerType, const Params &params)
throw (MediaServerException)
{
  GST_WARNING ("TODO: complete");
  return NULL;
}


MediaPipeline::StaticConstructor MediaPipeline::staticConstructor;

MediaPipeline::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
