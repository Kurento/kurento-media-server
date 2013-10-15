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
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "KmsMediaPlayerEndPointType_constants.h"
#include "PlayerEndPoint.hpp"
#include "KmsMediaRecorderEndPointType_constants.h"
#include "RecorderEndPoint.hpp"
#include "KmsMediaRtpEndPointType_constants.h"
#include "RtpEndPoint.hpp"
#include "KmsMediaZBarFilterType_constants.h"
#include "ZBarFilter.hpp"
#include "KmsMediaHttpEndPointType_constants.h"
#include "HttpEndPoint.hpp"

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

  this->objectType.__set_pipeline (*this);
}

MediaPipeline::MediaPipeline (const std::map<std::string, KmsMediaParam> &params) throw (KmsMediaServerException)
  : MediaObjectImpl (),
    KmsMediaPipeline ()
{
  if (params.empty () ) {
    init ();
  } else {
    throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_CONSTRUCTOR_NOT_FOUND,
        "MediaPipeline  only has the default constructor");
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
MediaPipeline::createMediaElement (const std::string &elementType, const std::map<std::string, KmsMediaParam>& params)
throw (KmsMediaServerException)
{
  GST_WARNING ("TODO: complete");

  if (g_KmsMediaPlayerEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    return std::shared_ptr<PlayerEndPoint> (new PlayerEndPoint (shared_from_this (), params) );
  } else if (g_KmsMediaRecorderEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    return std::shared_ptr<RecorderEndPoint> (new RecorderEndPoint (shared_from_this (), params) );
  } else if (g_KmsMediaRtpEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    return std::shared_ptr<RtpEndPoint> (new RtpEndPoint (shared_from_this (), params) );
  } else if (g_KmsMediaZBarFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    return std::shared_ptr<ZBarFilter> (new ZBarFilter (shared_from_this (), params) );
  } else if (g_KmsMediaHttpEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    return std::shared_ptr<HttpEndPoint> (new HttpEndPoint (shared_from_this (), params) );
  }

  throw createKmsMediaServerException (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_TYPE_NOT_FOUND,
      "There is not any media object type " + elementType);
}

std::shared_ptr<Mixer>
MediaPipeline::createMediaMixer (const std::string &mixerType, const std::map<std::string, KmsMediaParam>& params)
throw (KmsMediaServerException)
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
