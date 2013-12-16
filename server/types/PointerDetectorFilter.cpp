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

#include "PointerDetectorFilter.hpp"

#include "KmsMediaPointerDetectorFilterType_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"

#define GST_CAT_DEFAULT kurento_pointer_detector_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoPointerDetectorFilter"

#define WINDOWS_LAYOUT "windows-layout"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;

namespace kurento
{

void
pointerDetector_receive_message (GstBus *bus, GstMessage *message, gpointer pointerDetector)
{
  const GstStructure *st;
  gchar *windowID;
  const gchar *type;
  std::string windowIDStr, typeStr;
  PointerDetectorFilter *filter = (PointerDetectorFilter *) pointerDetector;

  if (GST_MESSAGE_SRC (message) != GST_OBJECT (filter->pointerDetector) ||
      GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
    return;

  st = gst_message_get_structure (message);
  type = gst_structure_get_name (st);

  if ( (g_strcmp0 (type, "window-out") != 0) &&
       (g_strcmp0 (type, "window-in") != 0) ) {
    GST_WARNING ("The message does not have the correct name");
    return;
  }

  if (!gst_structure_get (st, "window", G_TYPE_STRING , &windowID, NULL) ) {
    GST_WARNING ("The message does not contain the window ID");
    return;
  }

  windowIDStr = windowID;
  typeStr = type;

  g_free (windowID);

  filter->raiseEvent (typeStr, windowIDStr);

}

/* default constructor */
PointerDetectorFilter::PointerDetectorFilter (
  MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
  const std::map<std::string, KmsMediaParam> &params)
  : Filter (mediaSet, parent, g_KmsMediaPointerDetectorFilterType_constants.TYPE_NAME, params)
{
  const KmsMediaParam *p;
  KmsMediaPointerDetectorWindowSet windowSet;

  element = gst_element_factory_make ("filterelement", NULL);

  g_object_set (element, "filter-factory", "pointerdetector", NULL);
  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);

  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (parent->pipeline) );
  GstElement *pointerDetector;

  g_object_get (G_OBJECT (element), "filter", &pointerDetector, NULL);

  this->pointerDetector = pointerDetector;

  if (this->pointerDetector == NULL) {
    g_object_unref (bus);
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_NOT_AVAILAIBLE,
                                   "Media Object not available");
    throw except;
  }

  p = getParam (params,
                g_KmsMediaPointerDetectorFilterType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);

  if (p != NULL) {
    GstStructure *buttonsLayout;
    //there are data about windows
    unmarshalStruct (windowSet, p->data);
    /* set the window layout list */
    buttonsLayout = gst_structure_new_empty  ("windowsLayout");

    for (auto it = windowSet.windows.begin(); it != windowSet.windows.end(); ++it) {
      KmsMediaPointerDetectorWindow windowInfo = *it;
      GstStructure *buttonsLayoutAux;

      buttonsLayoutAux = gst_structure_new (
                           windowInfo.id.c_str(),
                           "upRightCornerX", G_TYPE_INT, windowInfo.topRightCornerX,
                           "upRightCornerY", G_TYPE_INT, windowInfo.topRightCornerY,
                           "width", G_TYPE_INT, windowInfo.width,
                           "height", G_TYPE_INT, windowInfo.height,
                           "id", G_TYPE_STRING, windowInfo.id.c_str(),
                           NULL);

      if (windowInfo.__isset.overlayImageUri) {
        gst_structure_set (buttonsLayoutAux, "uri",
                           G_TYPE_STRING, windowInfo.overlayImageUri.c_str(), NULL);
      }

      if (windowInfo.__isset.overlayTransparency) {
        gst_structure_set (buttonsLayoutAux, "transparency",
                           G_TYPE_DOUBLE, windowInfo.overlayTransparency, NULL);
      }

      gst_structure_set (buttonsLayout,
                         windowInfo.id.c_str(), GST_TYPE_STRUCTURE, buttonsLayoutAux,
                         NULL);

      gst_structure_free (buttonsLayoutAux);
    }

    g_object_set (G_OBJECT (this->pointerDetector), WINDOWS_LAYOUT, buttonsLayout, NULL);
    gst_structure_free (buttonsLayout);
  }

  windowSet.windows.clear();

  bus_handler_id = g_signal_connect (bus, "message", G_CALLBACK (pointerDetector_receive_message), this);
  g_object_unref (bus);
  // There is no need to reference pointerdetector because its life cycle is the same as the filter life cycle
  g_object_unref (pointerDetector);
}

PointerDetectorFilter::~PointerDetectorFilter() throw ()
{
  GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline) );
  g_signal_handler_disconnect (bus, bus_handler_id);
  g_object_unref (bus);

  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

void
PointerDetectorFilter::raiseEvent (const std::string &type, const std::string &windowID)
{
  KmsMediaEventData eventData;

  createStringEventData (eventData, windowID);

  if ( type.compare ("window-out") == 0) {
    GST_DEBUG ("Raise event. Type: %s, Window ID: %s", type.c_str(),
               windowID.c_str() );

    sendEvent (g_KmsMediaPointerDetectorFilterType_constants.EVENT_WINDOW_OUT, eventData);
  } else {
    GST_DEBUG ("Raise event. Type: %s, Window ID: %s", type.c_str(),
               windowID.c_str() );

    sendEvent (g_KmsMediaPointerDetectorFilterType_constants.EVENT_WINDOW_IN, eventData);
  }
}

void
PointerDetectorFilter::addWindow (KmsMediaPointerDetectorWindow window)
{
  GstStructure *buttonsLayout, *buttonsLayoutAux;

  buttonsLayoutAux = gst_structure_new (
                       window.id.c_str(),
                       "upRightCornerX", G_TYPE_INT, window.topRightCornerX,
                       "upRightCornerY", G_TYPE_INT, window.topRightCornerY,
                       "width", G_TYPE_INT, window.width,
                       "height", G_TYPE_INT, window.height,
                       "id", G_TYPE_STRING, window.id.c_str(),
                       NULL);

  if (window.__isset.overlayImageUri) {
    gst_structure_set (buttonsLayoutAux, "uri",
                       G_TYPE_STRING, window.overlayImageUri.c_str(), NULL);
  }

  if (window.__isset.overlayTransparency) {
    gst_structure_set (buttonsLayoutAux, "transparency",
                       G_TYPE_DOUBLE, window.overlayTransparency, NULL);
  }

  /* The function obtains the actual window list */
  g_object_get (G_OBJECT (pointerDetector), WINDOWS_LAYOUT, &buttonsLayout, NULL);
  gst_structure_set (buttonsLayout,
                     window.id.c_str(), GST_TYPE_STRUCTURE, buttonsLayoutAux,
                     NULL);

  g_object_set (G_OBJECT (pointerDetector), WINDOWS_LAYOUT, buttonsLayout, NULL);

  gst_structure_free (buttonsLayout);
  gst_structure_free (buttonsLayoutAux);
}

void
PointerDetectorFilter::removeWindow (std::string id)
{
  GstStructure *buttonsLayout;
  gint len;

  /* The function obtains the actual window list */
  g_object_get (G_OBJECT (pointerDetector), WINDOWS_LAYOUT, &buttonsLayout, NULL);
  len = gst_structure_n_fields (buttonsLayout);

  if (len == 0) {
    GST_WARNING ("There are no windows in the layout");
    return;
  }

  for (int i = 0; i < len; i++) {
    const gchar *name;
    name = gst_structure_nth_field_name (buttonsLayout, i);

    if ( g_strcmp0 (name, id.c_str() ) == 0) {
      /* this window will be removed */
      gst_structure_remove_field (buttonsLayout, name);
    }
  }

  /* Set the buttons layout list without the window with id = id */
  g_object_set (G_OBJECT (pointerDetector), WINDOWS_LAYOUT, buttonsLayout, NULL);

  gst_structure_free (buttonsLayout);
}

void
PointerDetectorFilter::clearWindows()
{
  GstStructure *buttonsLayout;

  buttonsLayout = gst_structure_new_empty  ("buttonsLayout");
  g_object_set (G_OBJECT (this->pointerDetector), WINDOWS_LAYOUT, buttonsLayout, NULL);
  gst_structure_free (buttonsLayout);
}

void
PointerDetectorFilter::invoke (KmsMediaInvocationReturn &_return,
                               const std::string &command,
                               const std::map< std::string, KmsMediaParam > &params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaPointerDetectorFilterType_constants.ADD_NEW_WINDOW.compare (command) == 0) {
    KmsMediaPointerDetectorWindow windowInfo;
    const KmsMediaParam *p;
    /* extract window params from param */
    p = getParam (params,
                  g_KmsMediaPointerDetectorFilterType_constants.ADD_NEW_WINDOW_PARAM_WINDOW);

    if (p != NULL) {
      unmarshalStruct (windowInfo, p->data);
      /* create window */
      addWindow (windowInfo);
    }
  } else if (g_KmsMediaPointerDetectorFilterType_constants.REMOVE_WINDOW.compare (command) == 0) {
    std::string id;

    getStringParam (id, params, g_KmsMediaPointerDetectorFilterType_constants.REMOVE_WINDOW_PARAM_WINDOW_ID);
    removeWindow (id);
  } else if (g_KmsMediaPointerDetectorFilterType_constants.CLEAR_WINDOWS.compare (command) == 0) {
    clearWindows();
  }
}

void
PointerDetectorFilter::subscribe (std::string &_return, const std::string &eventType,
                                  const std::string &handlerAddress,
                                  const int32_t handlerPort)
throw (KmsMediaServerException)
{
  if (g_KmsMediaPointerDetectorFilterType_constants.EVENT_WINDOW_IN == eventType)
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress, handlerPort);
  else  if (g_KmsMediaPointerDetectorFilterType_constants.EVENT_WINDOW_OUT == eventType)
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress, handlerPort);
  else
    Filter::subscribe (_return, eventType, handlerAddress, handlerPort);
}

PointerDetectorFilter::StaticConstructor PointerDetectorFilter::staticConstructor;

PointerDetectorFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
