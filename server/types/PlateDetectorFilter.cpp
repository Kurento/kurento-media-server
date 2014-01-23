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

#include "PlateDetectorFilter.hpp"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_plate_detector_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoPlateDetectorFilter"

namespace kurento
{

void
plate_detector_receive_message (GstBus *bus, GstMessage *message,
                                gpointer plateDetector)
{
  const GstStructure *st;
  gchar *plateNumber;
  const gchar *type;
  std::string typeStr, plateNumberStr;
  PlateDetectorFilter *filter = (PlateDetectorFilter *) plateDetector;

  if (GST_MESSAGE_SRC (message) != GST_OBJECT (filter->plateDetector) ||
      GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT) {
    return;
  }

  st = gst_message_get_structure (message);
  type = gst_structure_get_name (st);

  if (g_strcmp0 (type, "plate-detected") != 0) {
    GST_WARNING ("The message does not have the correct type");
    return;
  }

  if (! (gst_structure_get (st, "plate", G_TYPE_STRING , &plateNumber, NULL) ) ) {
    GST_WARNING ("The message does not contain the plate number");
    return;
  }

  plateNumberStr = plateNumber;
  typeStr = type;
  g_free (plateNumber);
  filter->raiseEvent (typeStr, plateNumberStr);
}

/* default constructor */
PlateDetectorFilter::PlateDetectorFilter (
  MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
  const std::map<std::string, KmsMediaParam> &params)
  : Filter (mediaSet, parent,
            g_KmsMediaPlateDetectorFilterType_constants.TYPE_NAME, params)
{
  GstElement *plateDetector;
  GstBus *bus;

  g_object_set (element, "filter-factory", "platedetector", NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);

  bus = gst_pipeline_get_bus (GST_PIPELINE (parent->pipeline) );
  bus_handler_id = g_signal_connect (bus, "message",
                                     G_CALLBACK (plate_detector_receive_message ), this);
  g_object_unref (bus);

  g_object_get (G_OBJECT (element), "filter", &plateDetector, NULL);
  this->plateDetector = plateDetector;
  // There is no need to reference platedetector because its life cycle is the same as the filter life cycle
  g_object_unref (plateDetector);
}

PlateDetectorFilter::~PlateDetectorFilter() throw ()
{
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (
                                        std::dynamic_pointer_cast<MediaPipeline> (parent)->pipeline ) );
  g_signal_handler_disconnect (bus, bus_handler_id);
  g_object_unref (bus);

  gst_bin_remove (GST_BIN ( std::dynamic_pointer_cast<MediaPipeline>
                            (parent)->pipeline ), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

void
PlateDetectorFilter::raiseEvent (const std::string &type,
                                 const std::string &plateNumber)
{
  KmsMediaEventData eventData;

  createStringEventData (eventData, plateNumber);

  if ( type == "plate-detected") {
    GST_DEBUG ("Raise event. Type: %s, Plate Number: %s", type.c_str(),
               plateNumber.c_str() );
    sendEvent (g_KmsMediaPlateDetectorFilterType_constants.EVENT_PLATE_DETECTED,
               eventData);
  }
}

void
PlateDetectorFilter::subscribe (std::string &_return,
                                const std::string &eventType,
                                const std::string &handlerAddress,
                                const int32_t handlerPort)
throw (KmsMediaServerException)
{
  if (g_KmsMediaPlateDetectorFilterType_constants.EVENT_PLATE_DETECTED ==
      eventType) {
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  } else {
    Filter::subscribe (_return, eventType, handlerAddress, handlerPort);
  }
}

PlateDetectorFilter::StaticConstructor PlateDetectorFilter::staticConstructor;

PlateDetectorFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}
} // kurento
