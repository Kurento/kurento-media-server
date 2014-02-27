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
#include <module.hpp>

#include "utils/utils.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "KmsMediaHttpGetEndPointType_constants.h"
#include "HttpGetEndPoint.hpp"
#include "KmsMediaHttpPostEndPointType_constants.h"
#include "HttpPostEndPoint.hpp"
#include "KmsMediaJackVaderFilterType_constants.h"
#include "JackVaderFilter.hpp"
#include "KmsMediaPointerDetectorFilterType_constants.h"
#include "PointerDetectorFilter.hpp"
#include "KmsMediaPointerDetector2FilterType_constants.h"
#include "PointerDetector2Filter.hpp"
#include "KmsMediaPlateDetectorFilterType_constants.h"
#include "PlateDetectorFilter.hpp"
#include "KmsMediaFaceOverlayFilterType_constants.h"
#include "FaceOverlayFilter.hpp"
#include "KmsMediaGStreamerFilterType_constants.h"
#include "GStreamerFilter.hpp"
#include "KmsMediaErrorCodes_constants.h"

#include "KmsMediaDispatcherMixerType_constants.h"
#include "DispatcherMixer.hpp"

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
    gst_debug_bin_to_dot_file_with_ts (GST_BIN (m->pipeline),
                                       GST_DEBUG_GRAPH_SHOW_ALL, "error");
    gst_message_parse_error (message, &err, &dbg_info);
    m->sendError ("UNEXPECTED_ERROR", err->message,
                  g_KmsMediaErrorCodes_constants.UNEXPECTED_ERROR);
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
  g_signal_connect (bus, "message", G_CALLBACK (media_pipeline_receive_message),
                    (gpointer) this);
  g_object_unref (bus);

  this->objectType.__set_pipeline (*this);
  this->unregChilds = false;
}

MediaPipeline::MediaPipeline (std::map <std::string, KurentoModule *> &modules,
                              MediaSet &mediaSet,
                              const std::map < std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : MediaObjectParent (mediaSet, params, true),
    KmsMediaPipeline (), modules (modules)
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
MediaPipeline::createMediaElement (const std::string &elementType,
                                   const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
{
  std::shared_ptr<MediaElement> element;
  KurentoModule *module = NULL;

  try {
    module = modules.at (elementType);
  } catch (std::out_of_range) {
  }

  if (module != NULL && module->type == KURENTO_MODULE_ELEMENT) {
    std::shared_ptr<MediaObjectImpl> obj = module->create_object (getMediaSet(),
                                           shared_from_this (),
                                           params);
    element = std::dynamic_pointer_cast<MediaElement> (obj);
  } else if (g_KmsMediaHttpPostEndPointType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<HttpPostEndPoint> (new HttpPostEndPoint (
                getMediaSet(),
                shared_from_this (), params) );
  } else if (g_KmsMediaHttpGetEndPointType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<HttpGetEndPoint> (new HttpGetEndPoint (getMediaSet(),
              shared_from_this (), params) );
  } else if (g_KmsMediaJackVaderFilterType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<JackVaderFilter> (new JackVaderFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaPointerDetectorFilterType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<PointerDetectorFilter> (new PointerDetectorFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaPlateDetectorFilterType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<PlateDetectorFilter> (new PlateDetectorFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaFaceOverlayFilterType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<FaceOverlayFilter> (new FaceOverlayFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaGStreamerFilterType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<GStreamerFilter> (new GStreamerFilter (
                getMediaSet(), shared_from_this (), params) );
  } else if (g_KmsMediaPointerDetector2FilterType_constants.TYPE_NAME.compare (
               elementType) == 0) {
    element = std::shared_ptr<PointerDetector2Filter> (new PointerDetector2Filter (
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
MediaPipeline::createMediaMixer (const std::string &mixerType,
                                 const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
{
  std::shared_ptr<Mixer> mixer;

  if (g_KmsMediaDispatcherMixerType_constants.TYPE_NAME.compare (
        mixerType) == 0) {
    mixer = std::shared_ptr<DispatcherMixer> (new DispatcherMixer (
              getMediaSet(), shared_from_this (), params) );
  } else {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_TYPE_NOT_FOUND,
                                   "There is not any media mixer type " + mixerType);
    throw except;
  }

  registerChild (mixer);
  return mixer;
}

MediaPipeline::StaticConstructor MediaPipeline::staticConstructor;

MediaPipeline::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
