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

  // There is no need to reference crowddetector because its life cycle is the same as the filter life cycle
  g_object_unref (crowdDetector);
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
