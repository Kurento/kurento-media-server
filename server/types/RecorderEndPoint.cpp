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

#include "RecorderEndPoint.hpp"

#include "RecorderEndPointType_constants.h"
#include "dataTypes_constants.h"
#include "errorCodes_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#define GST_CAT_DEFAULT kurento_recorder_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoRecorderEndPoint"

namespace kurento
{

void
RecorderEndPoint::init (std::shared_ptr<MediaPipeline> parent, const std::string &uri)
{
  element = gst_element_factory_make ("recorderendpoint", NULL);

  g_object_set (G_OBJECT (element), "uri", uri.c_str(), NULL);

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
}

RecorderEndPoint::RecorderEndPoint (std::shared_ptr<MediaPipeline> parent, const Params &params)
throw (MediaServerException)
  : UriEndPoint (parent, g_RecorderEndPointType_constants.TYPE_NAME)
{
  if (g_dataTypes_constants.STRING_DATA_TYPE.compare (params.dataType) == 0) {
    std::string uri;
    uri = unmarshalString (params.data);
    init (parent, uri);
  } else {
    throw createMediaServerException  (g_errorCodes_constants.MEDIA_OBJECT_CONSTRUCTOR_NOT_FOUND,
        "RecorderEndPoint has not any constructor with params of type " + params.dataType);
  }
}

RecorderEndPoint::~RecorderEndPoint() throw ()
{
  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

RecorderEndPoint::StaticConstructor RecorderEndPoint::staticConstructor;

RecorderEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
