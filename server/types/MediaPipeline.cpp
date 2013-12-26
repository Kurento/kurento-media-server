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
#include "KmsMediaHttpEndPointType_constants.h"
#include "HttpEndPoint.hpp"
#include "KmsMediaZBarFilterType_constants.h"
#include "ZBarFilter.hpp"
#include "KmsMediaJackVaderFilterType_constants.h"
#include "JackVaderFilter.hpp"
#include "KmsMediaPointerDetectorFilterType_constants.h"
#include "PointerDetectorFilter.hpp"
#include "KmsMediaWebRtcEndPointType_constants.h"
#include "WebRtcEndPoint.hpp"
#include "KmsMediaPlateDetectorFilterType_constants.h"
#include "PlateDetectorFilter.hpp"
#include "KmsMediaFaceOverlayFilterType_constants.h"
#include "FaceOverlayFilter.hpp"
#include "KmsMediaGStreamerFilterType_constants.h"
#include "GStreamerFilter.hpp"
#include "KmsMediaChromaFilterType_constants.h"
#include "ChromaFilter.hpp"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_media_pipeline
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaPipeline"

namespace kurento
{

void
media_pipeline_receive_message (GstBus *bus, GstMessage *message, gpointer data)
{
  MediaPipeline *m = (MediaPipeline *) data;

  switch (message->type) {
  case GST_MESSAGE_ERROR: {
    GError *err = NULL;
    gchar *dbg_info = NULL;

    GST_ERROR ("Error on bus: %" GST_PTR_FORMAT, message);
    gst_debug_bin_to_dot_file_with_ts (GST_BIN (m->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "error");
    gst_message_parse_error (message, &err, &dbg_info);
    m->sendError ("UNEXPECTED_ERROR", err->message, g_KmsMediaErrorCodes_constants.UNEXPECTED_ERROR);
    g_error_free (err);
    g_free (dbg_info);
    break;
  }

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
  g_signal_connect (bus, "message", G_CALLBACK (media_pipeline_receive_message), (gpointer) this);
  g_object_unref (bus);

  this->objectType.__set_pipeline (*this);
}

MediaPipeline::MediaPipeline (MediaSet &mediaSet,
                              const std::map < std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : MediaObjectParent (mediaSet, params),
    KmsMediaPipeline ()
{
  init ();
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
MediaPipeline::createMediaElement (const std::string &elementType, const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
{
  std::shared_ptr<MediaElement> element;

  if (g_KmsMediaPlayerEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<PlayerEndPoint> (new PlayerEndPoint (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaRecorderEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<RecorderEndPoint> (new RecorderEndPoint (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaRtpEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<RtpEndPoint> (new RtpEndPoint (getMediaSet(),
                                            shared_from_this (), params) );
  } else if (g_KmsMediaHttpEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<HttpEndPoint> (new HttpEndPoint (getMediaSet(),
              shared_from_this (), params) );
  } else if (g_KmsMediaZBarFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<ZBarFilter> (new ZBarFilter (getMediaSet(),
                                           shared_from_this (), params) );
  } else if (g_KmsMediaJackVaderFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<JackVaderFilter> (new JackVaderFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaPointerDetectorFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<PointerDetectorFilter> (new PointerDetectorFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaPlateDetectorFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<PlateDetectorFilter> (new PlateDetectorFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaWebRtcEndPointType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<WebRtcEndPoint> (new WebRtcEndPoint (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaFaceOverlayFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<FaceOverlayFilter> (new FaceOverlayFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaGStreamerFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<GStreamerFilter> (new GStreamerFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaChromaFilterType_constants.TYPE_NAME.compare (elementType) == 0) {
    element = std::shared_ptr<ChromaFilter> (new ChromaFilter (
                getMediaSet(), shared_from_this (), params) );
  } else {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_TYPE_NOT_FOUND,
                                   "There is not any media object type " + elementType);
    throw except;
  }

  registerChild (element);
  return element;
}

std::shared_ptr<Mixer>
MediaPipeline::createMediaMixer (const std::string &mixerType, const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
{
  KmsMediaServerException except;

  GST_WARNING ("TODO: complete");
  createKmsMediaServerException (except,
                                 g_KmsMediaErrorCodes_constants.NOT_IMPLEMENTED,
                                 "Not implemented");
  throw except;
}

MediaPipeline::StaticConstructor MediaPipeline::staticConstructor;

MediaPipeline::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
