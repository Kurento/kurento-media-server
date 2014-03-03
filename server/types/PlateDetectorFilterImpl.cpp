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

#include "PlateDetectorFilterImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include "MediaPipelineImpl.hpp"
#include <KurentoException.hpp>

#define GST_CAT_DEFAULT kurento_plate_detector_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoPlateDetectorFilterImpl"


namespace kurento
{

static void
bus_message_adaptor (GstBus *bus, GstMessage *message, gpointer data)
{
  auto func = reinterpret_cast<std::function<void (GstMessage *message) >*>
              (data);

  (*func) (message);
}

PlateDetectorFilterImpl::PlateDetectorFilterImpl (
  std::shared_ptr< MediaObjectImpl > parent, int garbagePeriod) :
  FilterImpl (parent, garbagePeriod)
{
  GstBus *bus;
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );

  g_object_set (element, "filter-factory", "platedetector", NULL);

  g_object_get (G_OBJECT (element), "filter", &plateDetector, NULL);

  if (plateDetector == NULL) {
    throw KurentoException ("Media Object not available");
  }

  // There is no need to reference platedetector because its life cycle is the same as the filter life cycle
  g_object_unref (plateDetector);

  busMessageLambda = [&] (GstMessage * message) {
    const GstStructure *st;
    gchar *plateNumber;
    const gchar *type;
    std::string typeStr, plateNumberStr;

    if (GST_MESSAGE_SRC (message) != GST_OBJECT (plateDetector) ||
        GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT) {
      return;
    }

    st = gst_message_get_structure (message);
    type = gst_structure_get_name (st);

    if (g_strcmp0 (type, "plate-detected") != 0) {
      GST_WARNING ("The message does not have the correct type");
      return;
    }

    if (! (gst_structure_get (st, "plate", G_TYPE_STRING , &plateNumber,
                              NULL) ) ) {
      GST_WARNING ("The message does not contain the plate number");
      return;
    }

    plateNumberStr = plateNumber;
    typeStr = type;

    PlateDetected event (plateNumberStr, shared_from_this(), typeStr);
    signalPlateDetected (event);

    g_free (plateNumber);
  };

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipe->getPipeline() ) );
  bus_handler_id = g_signal_connect (bus, "message",
                                     G_CALLBACK (bus_message_adaptor),
                                     &busMessageLambda);
  g_object_unref (bus);
}

PlateDetectorFilterImpl::~PlateDetectorFilterImpl()
{
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipe->getPipeline() ) );
  g_signal_handler_disconnect (bus, bus_handler_id);
  g_object_unref (bus);
}

std::shared_ptr<MediaObject>
PlateDetectorFilter::Factory::createObject (std::shared_ptr<MediaPipeline>
    mediaPipeline,
    int garbagePeriod)
{
  std::shared_ptr<MediaObject> object (new PlateDetectorFilterImpl (
                                         std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline),
                                         garbagePeriod) );

  return object;
}

PlateDetectorFilterImpl::StaticConstructor
PlateDetectorFilterImpl::staticConstructor;

PlateDetectorFilterImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
