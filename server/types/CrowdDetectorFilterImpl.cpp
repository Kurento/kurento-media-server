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

#include "CrowdDetectorFilterImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include <generated/RegionOfInterest.hpp>
#include <generated/Point.hpp>
#include <generated/RegionOfInterestConfig.hpp>
#include "MediaPipelineImpl.hpp"
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_crowd_detector_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoCrowdDetectorFilterImpl"

#define ROIS_PARAM "rois"

namespace kurento
{

static void
bus_message_adaptor (GstBus *bus, GstMessage *message, gpointer data)
{
  auto func = reinterpret_cast<std::function<void (GstMessage *message) >*>
              (data);

  (*func) (message);
}

static GstStructure *
get_structure_from_roi (std::shared_ptr<RegionOfInterest> roi)
{
  GstStructure *roiStructure, *configRoiSt;
  std::shared_ptr<RegionOfInterestConfig> config;
  int pointCount = 0;

  roiStructure = gst_structure_new_empty (roi->getId().c_str() );

  for (std::shared_ptr<Point> point : roi->getPoints() ) {
    GstStructure *pointSt;
    std::string name = "point" + std::to_string (pointCount ++);

    pointSt = gst_structure_new (name.c_str(),
                                 "x", G_TYPE_INT, point->getX(),
                                 "y", G_TYPE_INT, point->getY(),
                                 NULL);

    gst_structure_set (roiStructure,
                       name.c_str(), GST_TYPE_STRUCTURE, pointSt,
                       NULL);

    gst_structure_free (pointSt);
  }

  config = roi->getRegionOfInterestConfig();
  configRoiSt = gst_structure_new ("config",
                                   "id", G_TYPE_STRING, roi->getId().c_str(),
                                   "occupancy_level_min", G_TYPE_INT, config->getOccupancyLevelMin(),
                                   "occupancy_level_med", G_TYPE_INT, config->getOccupancyLevelMed(),
                                   "occupancy_level_max", G_TYPE_INT, config->getOccupancyLevelMax(),
                                   "occupancy_num_frames_to_event", G_TYPE_INT,
                                   config->getOccupancyNumFramesToEvent(),
                                   "fluidity_level_min", G_TYPE_INT, config->getFluidityLevelMin(),
                                   "fluidity_level_med", G_TYPE_INT, config->getFluidityLevelMed(),
                                   "fluidity_level_max", G_TYPE_INT, config->getFluidityLevelMax(),
                                   "fluidity_num_frames_to_event", G_TYPE_INT,
                                   config->getFluidityNumFramesToEvent(),
                                   "send_optical_flow_event", G_TYPE_BOOLEAN, config->getSendOpticalFlowEvent(),
                                   "optical_flow_num_frames_to_event", G_TYPE_INT,
                                   config->getOpticalFlowNumFramesToEvent(),
                                   "optical_flow_num_frames_to_reset", G_TYPE_INT,
                                   config->getOpticalFlowNumFramesToReset(),
                                   "optical_flow_angle_offset", G_TYPE_INT, config->getOpticalFlowAngleOffset(),
                                   NULL);
  gst_structure_set (roiStructure,
                     "config", GST_TYPE_STRUCTURE, configRoiSt,
                     NULL);

  gst_structure_free (configRoiSt);

  return roiStructure;
}

CrowdDetectorFilterImpl::CrowdDetectorFilterImpl (
  const std::vector<std::shared_ptr<RegionOfInterest>> &rois,
  std::shared_ptr< MediaObjectImpl > parent) :
  FilterImpl (parent)
{
  GstBus *bus;
  std::shared_ptr<MediaPipelineImpl> pipe;
  GstStructure *roisStructure;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );

  g_object_set (element, "filter-factory", "crowddetector", NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipe->getPipeline() ) );

  g_object_get (G_OBJECT (element), "filter", &crowdDetector, NULL);

  if (crowdDetector == NULL) {
    g_object_unref (bus);
    throw KurentoException (MEDIA_OBJECT_NOT_AVAILABLE,
                            "Media Object not available");
  }

  roisStructure = gst_structure_new_empty  ("Rois");

  for (auto roi : rois) {
    GstStructure *roiStructureAux = get_structure_from_roi (roi);

    gst_structure_set (roisStructure,
                       roi->getId().c_str(), GST_TYPE_STRUCTURE,
                       roiStructureAux,
                       NULL);

    gst_structure_free (roiStructureAux);
  }

  g_object_set (G_OBJECT (crowdDetector), ROIS_PARAM, roisStructure, NULL);
  gst_structure_free (roisStructure);

  busMessageLambda = [&] (GstMessage * message) {
    const GstStructure *st;
    gchar *roiID;
    const gchar *type;
    std::string roiIDStr, typeStr;

    if (GST_MESSAGE_SRC (message) != GST_OBJECT (crowdDetector) ||
        GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT) {
      return;
    }

    st = gst_message_get_structure (message);
    type = gst_structure_get_name (st);

    if (!gst_structure_get (st, "roi", G_TYPE_STRING , &roiID, NULL) ) {
      GST_WARNING ("The message does not contain the roi ID");
      return;
    }

    roiIDStr = roiID;
    typeStr = type;

    g_free (roiID);

    if (typeStr == "fluidity-event") {

      double fluidity_percentage;
      int fluidity_level;

      if (! (gst_structure_get (st, "fluidity_percentage", G_TYPE_DOUBLE,
                                &fluidity_percentage, NULL) ) ) {
        GST_WARNING ("The message does not contain the fluidity percentage");
        return;
      }

      if (! (gst_structure_get (st, "fluidity_level", G_TYPE_INT,
                                &fluidity_level, NULL) ) ) {
        GST_WARNING ("The message does not contain the fluidity level");
        return;
      }

      // TODO: FluidityEvent event (roiIDStr, shared_from_this(),
      //                            FluidityEvent::getName() );
      // TODO: signalFluidityEvent (event);
    } else if (typeStr == "occupancy-event") {

      double occupancy_percentage;
      int occupancy_level;

      if (! (gst_structure_get (st, "occupancy_percentage", G_TYPE_DOUBLE,
                                &occupancy_percentage, NULL) ) ) {
        GST_WARNING ("The message does not contain the occupancy percentage");
        return;
      }

      if (! (gst_structure_get (st, "occupancy_level", G_TYPE_INT,
                                &occupancy_level, NULL) ) ) {
        GST_WARNING ("The message does not contain the occupancy level");
        return;
      }

      // TODO: OccupancyEvent event (roiIDStr, shared_from_this(),
      //                             OccupancyEvent::getName() );
      // TODO: signalOccupancyEvent (event);
    } else if (typeStr == "direction-event") {

      double direction_angle;

      if (! (gst_structure_get (st, "direction_angle", G_TYPE_DOUBLE,
                                &direction_angle, NULL) ) ) {
        GST_WARNING ("The message does not contain the direction angle");
        return;
      }

      // TODO: DirectionEvent event (roiIDStr, shared_from_this(),
      //                            DirectionEvent::getName() );
      // TODO: signalDirectionEvent (event);
    } else {
      GST_WARNING ("The message does not have the correct name");
    }
  };

  bus_handler_id = g_signal_connect (bus, "message",
                                     G_CALLBACK (bus_message_adaptor),
                                     &busMessageLambda);
  g_object_unref (bus);

  // There is no need to reference crowddetector because its life cycle is the same as the filter life cycle
  g_object_unref (crowdDetector);
}

CrowdDetectorFilterImpl::~CrowdDetectorFilterImpl()
{
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipe->getPipeline() ) );
  g_signal_handler_disconnect (bus, bus_handler_id);
  g_object_unref (bus);
}

MediaObject *
CrowdDetectorFilter::Factory::createObject (
  std::shared_ptr<MediaPipeline> mediaPipeline,
  const std::vector<std::shared_ptr<RegionOfInterest>> &rois)
{
  return new CrowdDetectorFilterImpl (rois,
                                      std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline) );
}

CrowdDetectorFilterImpl::StaticConstructor
CrowdDetectorFilterImpl::staticConstructor;

CrowdDetectorFilterImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
