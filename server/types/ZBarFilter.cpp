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

#include "ZBarFilter.hpp"

#include "KmsMediaZBarFilterType_constants.h"

#include "utils/utils.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

#define GST_CAT_DEFAULT kurento_zbar_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoZBarFilter"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

void
zbar_receive_message (GstBus *bus, GstMessage *message, gpointer zbar)
{
  ZBarFilter *filter = (ZBarFilter *) zbar;

  if (GST_MESSAGE_SRC (message) == GST_OBJECT (filter->zbar) &&
      GST_MESSAGE_TYPE (message) == GST_MESSAGE_ELEMENT) {
    const GstStructure *st;
    guint64 ts;
    gchar *type, *symbol;

    st = gst_message_get_structure (message);

    if (g_strcmp0 (gst_structure_get_name (st), "barcode") != 0) {
      return;
    }

    if (!gst_structure_get (st, "timestamp", G_TYPE_UINT64, &ts,
                            "type", G_TYPE_STRING, &type, "symbol", G_TYPE_STRING, &symbol, NULL) ) {
      return;
    }

    std::string symbolStr (symbol);
    std::string typeStr (type);

    g_free (type);
    g_free (symbol);

    filter->barcodeDetected (ts, typeStr, symbolStr);
  }
}

void
ZBarFilter::init (std::shared_ptr<MediaPipeline> parent)
{
  g_object_set (element, "filter-factory", "zbar", NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);

  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (parent->pipeline) );
  GstElement *zbar;

  g_object_get (G_OBJECT (element), "filter", &zbar, NULL);

  this->zbar = zbar;
  g_object_set (G_OBJECT (zbar), "qos", FALSE, NULL);

  bus_handler_id = g_signal_connect (bus, "message",
                                     G_CALLBACK (zbar_receive_message), this);
  g_object_unref (bus);
  // There is no need to reference zbar becase its live cycle is the same as the filter live cycle
  g_object_unref (zbar);
}

ZBarFilter::ZBarFilter (MediaSet &mediaSet,
                        std::shared_ptr<MediaPipeline> parent,
                        const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : Filter (mediaSet, parent, g_KmsMediaZBarFilterType_constants.TYPE_NAME,
            params)
{
  init (parent);
}

ZBarFilter::~ZBarFilter() throw ()
{
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (
                                        std::dynamic_pointer_cast<MediaPipeline> (parent)->pipeline) );

  g_signal_handler_disconnect (bus, bus_handler_id);
  g_object_unref (bus);

  gst_bin_remove (GST_BIN ( std::dynamic_pointer_cast<MediaPipeline>
                            (parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

void
ZBarFilter::raiseEvent (guint64 ts, std::string &type, std::string &symbol)
{
  KmsMediaEventData eventData;
  KmsMediaEventCodeFoundData codeFoundData;
  std::string codeFoundDataStr;

  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);

  codeFoundData.__set_type (type);
  codeFoundData.__set_value (symbol);
  codeFoundData.write (&protocol);
  transport->appendBufferToString (codeFoundDataStr);

  eventData.__set_dataType (
    g_KmsMediaZBarFilterType_constants.EVENT_CODE_FOUND_DATA_TYPE);
  eventData.__set_data (codeFoundDataStr);

  GST_DEBUG ("Raise event");
  GST_DEBUG ("Time stamp: %" G_GUINT64_FORMAT, ts);
  GST_DEBUG ("Type: %s", type.c_str() );
  GST_DEBUG ("Symbol: %s", symbol.c_str() );

  sendEvent (g_KmsMediaZBarFilterType_constants.EVENT_CODE_FOUND, eventData);
}

void
ZBarFilter::barcodeDetected (guint64 ts, std::string &type, std::string &symbol)
{
  if (lastSymbol != symbol || lastType != type ||
      lastTs == G_GUINT64_CONSTANT (0) || ( (ts - lastTs) >= GST_SECOND) ) {
    lastSymbol = symbol;
    lastType = type;
    lastTs = ts;
    raiseEvent (ts, type, symbol);
  }
}

void
ZBarFilter::subscribe (std::string &_return, const std::string &eventType,
                       const std::string &handlerAddress,
                       const int32_t handlerPort) throw (KmsMediaServerException)
{
  if (g_KmsMediaZBarFilterType_constants.EVENT_CODE_FOUND == eventType) {
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  } else {
    Filter::subscribe (_return, eventType, handlerAddress, handlerPort);
  }
}

ZBarFilter::StaticConstructor ZBarFilter::staticConstructor;

ZBarFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
