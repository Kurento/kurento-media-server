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

#include "ZBarFilterImpl.hpp"
#include <generated/MediaPipeline.hpp>
#include <MediaPipelineImpl.hpp>

#define GST_CAT_DEFAULT kurento_z_bar_filter_impl
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoZBarFilterImpl"


namespace kurento
{

static void
bus_message_adaptor (GstBus *bus, GstMessage *message, gpointer data)
{
  auto func = reinterpret_cast<std::function<void (GstMessage *message) >*>
              (data);

  (*func) (message);
}

ZBarFilterImpl::ZBarFilterImpl (
  std::shared_ptr< MediaObjectImpl > parent, int garbagePeriod) :
  FilterImpl (parent, garbagePeriod)
{
  GstBus *bus;
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipe->getPipeline() ) );

  g_object_set (element, "filter-factory", "zbar", NULL);
  g_object_get (G_OBJECT (element), "filter", &zbar, NULL);

  if (zbar == NULL) {
  }

  g_object_set (G_OBJECT (zbar), "qos", FALSE, NULL);

  busMessageLambda = [&] (GstMessage * message) {
    if (GST_MESSAGE_SRC (message) == GST_OBJECT (zbar) &&
        GST_MESSAGE_TYPE (message) == GST_MESSAGE_ELEMENT) {
      const GstStructure *st;
      guint64 ts;
      gchar *type, *symbol;

      st = gst_message_get_structure (message);

      if (g_strcmp0 (gst_structure_get_name (st), "barcode") != 0) {
        return;
      }

      if (!gst_structure_get (st, "timestamp", G_TYPE_UINT64, &ts,
                              "type", G_TYPE_STRING, &type, "symbol",
                              G_TYPE_STRING, &symbol, NULL) ) {
        return;
      }

      std::string symbolStr (symbol);
      std::string typeStr (type);

      g_free (type);
      g_free (symbol);

      barcodeDetected (ts, typeStr, symbolStr);
    }
  };

  bus_handler_id = g_signal_connect (bus, "message",
                                     G_CALLBACK (bus_message_adaptor),
                                     &busMessageLambda);
  g_object_unref (bus);
  // There is no need to reference zbar becase its live cycle is the same as the filter live cycle
  g_object_unref (zbar);
}

void
ZBarFilterImpl::barcodeDetected (guint64 ts, std::string &type,
                                 std::string &symbol)
{
  if (lastSymbol != symbol || lastType != type ||
      lastTs == G_GUINT64_CONSTANT (0) || ( (ts - lastTs) >= GST_SECOND) ) {
    // TODO: Hold a lock here to avoid race conditions

    lastSymbol = symbol;
    lastType = type;
    lastTs = ts;

    CodeFound event (type, symbol, shared_from_this(), CodeFound::getName() );
    signalCodeFound (event);
  }
}

std::shared_ptr<MediaObject>
ZBarFilter::Factory::createObject (std::shared_ptr<MediaPipeline> mediaPipeline,
                                   int garbagePeriod)
{
  std::shared_ptr<MediaObject> object (new ZBarFilterImpl (
                                         std::dynamic_pointer_cast<MediaObjectImpl> (mediaPipeline),
                                         garbagePeriod) );

  return object;
}

ZBarFilterImpl::~ZBarFilterImpl()
{
  std::shared_ptr<MediaPipelineImpl> pipe;

  pipe = std::dynamic_pointer_cast<MediaPipelineImpl> (getMediaPipeline() );
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipe->getPipeline() ) );
  g_signal_handler_disconnect (bus, bus_handler_id);
  g_object_unref (bus);
}

ZBarFilterImpl::StaticConstructor ZBarFilterImpl::staticConstructor;

ZBarFilterImpl::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */
