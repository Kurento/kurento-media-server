/*
 * ZBarFilter.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ZBarFilter.hpp"

#define GST_CAT_DEFAULT kurento_zbar_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoZBarFilter"

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

    if (g_strcmp0 (gst_structure_get_name (st), "barcode") != 0)
      return;

    if (!gst_structure_get (st, "timestamp", G_TYPE_UINT64, &ts,
        "type", G_TYPE_STRING, &type, "symbol", G_TYPE_STRING, &symbol, NULL) )
      return;

    std::string symbolStr (symbol);
    std::string typeStr (type);

    g_free (type);
    g_free (symbol);

    filter->barcodeDetected (ts, typeStr, symbolStr);
  }
}

ZBarFilter::ZBarFilter (std::shared_ptr<MediaPipeline> parent) : Filter (parent, FilterType::type::ZBAR_FILTER)
{
  gchar *name;

  name = getIdStr ();
  element = gst_element_factory_make ("filterelement", name);
  g_free (name);

  g_object_set (element, "filter-factory", "zbar", NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);

  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (parent->pipeline) );
  GstElement *zbar;

  g_object_get (G_OBJECT (element), "filter", &zbar, NULL);

  this->zbar = zbar;

  bus_handler_id = g_signal_connect (bus, "message", G_CALLBACK (zbar_receive_message), this);
  g_object_unref (bus);
  // There is no need to reference zbar becase its live cycle is the same as the filter live cycle
  g_object_unref (zbar);
}

ZBarFilter::~ZBarFilter() throw ()
{
  g_source_remove (bus_handler_id);

  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

void
ZBarFilter::barcodeDetected (guint64 ts, std::string &type, std::string &symbol)
{
  if (lastSymbol != symbol || lastType != type ||
      lastTs == G_GUINT64_CONSTANT (0) || ( (ts - lastTs) >= GST_SECOND) ) {
    lastSymbol = symbol;
    lastType = type;
    lastTs = ts;

    // TODO: Raise event

    GST_DEBUG ("Raise event");
    GST_DEBUG ("Time stamp: %" G_GUINT64_FORMAT, ts);
    GST_DEBUG ("Type: %s", type.c_str() );
    GST_DEBUG ("Symbol: %s", symbol.c_str() );
  }
}


ZBarFilter::StaticConstructor ZBarFilter::staticConstructor;

ZBarFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
